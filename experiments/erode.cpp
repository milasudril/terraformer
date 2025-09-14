//@	{"target":{"name":"erode.o"}}

#include "lib/common/move_only_function.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/math_utils/butter_lp_2d.hpp"
#include "lib/math_utils/computation_context.hpp"
#include "lib/math_utils/filter_utils.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/execution/thread_pool.hpp"
#include "lib/execution/signaling_counter.hpp"
#include "lib/execution/batch_result.hpp"
#include "lib/array_classes/single_array.hpp"

#include <algorithm>
#include <random>
#include <bit>

using thread_pool_type = terraformer::thread_pool<terraformer::move_only_function<void()>>;

[[nodiscard]] float erode(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	terraformer::span_2d<float const> noise,
	float maxval_in,
	uint32_t input_y_offset = 0
)
{
	using clamp_tag = terraformer::span_2d_extents::clamp_tag;
	auto maxval = 0.0f;

	for(int32_t y = 0; y != static_cast<int32_t>(output.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(output.width()); ++x)
		{
			auto const y_in = y + input_y_offset;
			auto const ddx_input = input(x + 1, y_in, clamp_tag{}) - input(x - 1, y_in, clamp_tag{});
			auto const ddy_input = input(x, y_in + 1, clamp_tag{}) - input(x, y_in - 1, clamp_tag{});
			terraformer::displacement grad_z{ddx_input, ddy_input, 0.0f};
			auto const grad_size = norm(grad_z);
			if(grad_size  == 0.0f)
			{ continue; }


			terraformer::location current_loc{static_cast<float>(x), static_cast<float>(y_in), 0.0f};

			auto const input_val = input(x, y_in);

			auto const sample_from = current_loc - grad_z/grad_size;
			auto const downhill_value = terraformer::interp(
				input,
				sample_from[0],
				sample_from[1],
				terraformer::clamp_at_boundary{}
			);

			auto const noise_val = noise(x ,y_in);
			auto const minval = std::min(input_val, downhill_value);

			auto const val = std::lerp(
				input_val,
				minval,
				input_val*noise_val*noise_val/maxval_in
			);
			maxval = std::max(val, maxval);

			output(x, y) = val;
		}
	}
	return maxval;
}

[[nodiscard]] terraformer::batch_result<float> erode(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	terraformer::span_2d<float const> noise,
	float maxval_in,
	thread_pool_type& workers
)
{
	terraformer::batch_result<float> retvals{workers.max_concurrency()};
	auto const height = output.height();
	auto const n_workers = workers.max_concurrency();
	auto const batch_size = 1 + (height - 1)/static_cast<uint32_t>(n_workers);
	terraformer::single_array maxvals{terraformer::array_size<float>{n_workers}};

	for(auto k : maxvals.element_indices())
	{
		terraformer::scanline_range range{
			.begin = static_cast<uint32_t>(k.get()*batch_size),
			.end = static_cast<uint32_t>((k + 1).get()*batch_size)
		};

		workers.submit(
			[
				&retvals = retvals.get_state(),
				maxval_in,
				scanlines_out = output.scanlines(range),
				scanlines_in = input,
				scanlines_noise = noise,
				input_y_offset = range.begin
			](){
				retvals.save_partial_result(erode(scanlines_out, scanlines_in, scanlines_noise, maxval_in, input_y_offset));
			}
		);
	}
	return retvals;
}

void make_white_noise(terraformer::span_2d<float> output, terraformer::random_generator& rng)
{
	auto const width = output.width();
	auto const height = output.height();

	std::uniform_real_distribution input_intensity{0.0f, 1.0f};
	for(uint32_t y = 0;  y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{ output(x, y) = input_intensity(rng); }
	}
}

[[nodiscard]] terraformer::signaling_counter make_white_noise(
	terraformer::span_2d<float> output,
	thread_pool_type& workers,
	terraformer::span<terraformer::random_generator> rngs
)
{
	assert(workers.max_concurrency() == std::size(rngs).get());

	return terraformer::generate(
		output,
		workers,
		[]<class ... Args>(
			terraformer::scanline_generate_job const& jobinfo,
			terraformer::span_2d<float> output,
			terraformer::span<terraformer::random_generator> rngs
		){
			make_white_noise(output, rngs[rngs.element_indices().front() + jobinfo.chunk_index]);
		},
		rngs
	);
}

void accumulate(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	float factor,
	float input_gain
)
{
	auto const width = output.width();
	auto const height = output.height();
	for(uint32_t y = 0; y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{ output(x, y) = (1.0f - factor)*output(x, y) + factor*input(x, y)*input_gain; }
	}
}

[[nodiscard]] terraformer::signaling_counter accumulate(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	float factor,
	float input_gain,
	thread_pool_type& workers
)
{
	terraformer::signaling_counter counter{workers.max_concurrency()};
	auto const height = output.height();
	auto const n_workers = workers.max_concurrency();
	auto const batch_size = 1 + (height - 1)/static_cast<uint32_t>(n_workers);

	for(size_t k = 0; k != workers.max_concurrency(); ++k)
	{
		terraformer::scanline_range range{
			.begin = static_cast<uint32_t>(k*batch_size),
			.end = static_cast<uint32_t>((k + 1)*batch_size)
		};

		workers.submit(
			[
				&counter = counter.get_state(),
				factor,
				input_gain,
				scanlines_out = output.scanlines(range),
				scanlines_in = input.scanlines(range)
			](){
				accumulate(scanlines_out, scanlines_in, factor, input_gain);
				counter.decrement();
			}
		);
	}

	return counter;
}

struct linux_sched_params
{
	int policy;
	int priority;

	void apply(pthread_t thread) const
	{
		sched_param param{
			.sched_priority = priority
		};

		if(auto res = pthread_setschedparam(thread, policy, &param); res != 0)
		{
			errno = res;
			perror("Failed to set scheduling policy");
		}
	}
};

constexpr auto fold_minmax_value = [](auto const& range) {
	return std::ranges::min_max_result{
		.min = std::ranges::min_element(
			range,
			[](auto a, auto b){
				return a.min < b.min;
			}
		)->min,
		.max = std::ranges::max_element(
			range,
			[](auto a, auto b) {
				return a.max < b.max;
			}
		)->max
	};
};

int main(int argc, char** argv)
{
	if(argc < 3)
	{ return -1; }

	terraformer::random_generator rng;
	auto buffer_a = load(terraformer::empty<terraformer::grayscale_image>{}, argv[1]);
	auto buffer_b = buffer_a;

	//auto input = buffer_a.pixels();
	//auto output = buffer_b.pixels();

	auto white_noise_buffer = buffer_a;
	auto filtered_noise_buffer = buffer_a;
	auto accumulated_noise = buffer_a;
	auto filter_mask = buffer_a;


	pthread_attr_t attr{};
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 16384);

	auto const n_threads = std::thread::hardware_concurrency();

	terraformer::computation_context comp_ctxt{
		.workers = terraformer::thread_pool<terraformer::move_only_function<void()>>{
			std::thread::hardware_concurrency()
		},
		.dft_engine = terraformer::dft_engine{}
	};
	terraformer::dft_engine::enable_multithreading(comp_ctxt.workers);

	comp_ctxt.workers.set_schedparams(linux_sched_params{
		.policy = SCHED_BATCH,
		.priority = 0
	});

	auto pending_filter_mask = generate(
		filter_mask.pixels(),
		comp_ctxt.workers,
		[]<class ... Args>(Args&&... params){
			make_filter_mask(std::forward<Args>(params)...);
		},
		terraformer::butter_lp_2d_descriptor{
			.f_x = 49152.0f/8192.0f,
			.f_y = 49152.0f/8192.0f,
			.hf_rolloff = 2.0f,
			.y_direction = 0.0f
		}
	);
	terraformer::random_generator master_rng;
	terraformer::single_array<terraformer::random_generator> rngs;
	for(size_t k = 0; k != n_threads; ++k)
	{ rngs.push_back(terraformer::random_generator{terraformer::generate_rng_seed(master_rng)}); }

	auto pending_noise = make_white_noise(white_noise_buffer.pixels(), comp_ctxt.workers, rngs);


	pending_filter_mask.wait();
	pending_noise.wait();

	terraformer::apply_filter(
		white_noise_buffer.pixels(),
		filtered_noise_buffer.pixels(),
		comp_ctxt,
		filter_mask.pixels()
	).wait();

	auto minmaxval_filter = terraformer::dispatch_jobs(
		std::as_const(filtered_noise_buffer).pixels(),
		comp_ctxt.workers,
		[](
			size_t,
			uint32_t,
			uint32_t,
			terraformer::span_2d<float const> output
		){
			return minmax_value(output);
		}
	);

	terraformer::generate(
		filtered_noise_buffer.pixels(),
		comp_ctxt.workers,
		[](
			auto const&,
			terraformer::span_2d<float> output,
			std::ranges::minmax_result<float> input_range
		){
			return terraformer::normalize(output, input_range);
		},
		minmaxval_filter.get_result(fold_minmax_value)
	).wait();

	store(filtered_noise_buffer.pixels(), "/dev/shm/slask.exr");
#if 0

	for(size_t k = 0; k != 1024; ++k)
	{
		if(std::has_single_bit(k + 1))
		{
			std::filesystem::path filename{argv[2]};
			filename /= std::to_string(k) + ".exr";
			store(input, filename.c_str());
		}

		auto const maxval_erode = erode(output, input, accumulated_noise.pixels(), 3500.0f, workers);
		make_white_noise(white_noise_buffer.pixels(), workers, rngs).wait();
		maxval_filter = apply_lowpass_filter(filtered_noise_buffer.pixels(), white_noise_buffer.pixels(), workers);
		auto noise_accumulate_sem = accumulate(
			accumulated_noise.pixels(),
			filtered_noise_buffer.pixels(),
			0.25f,
			1.0f/maxval_filter.get_result(max_value),
			workers
		);
		auto output_amplify_sem = amplify(output, 3500.0f/maxval_erode.get_result(max_value), workers);
		std::swap(output, input);

		if(k %16 == 0)
		{
			printf("\r");
			auto count = static_cast<size_t>(64.0f*static_cast<float>(k)/65536.0f);
			for(size_t k = 0; k != count; ++k)
			{
				printf("=");
			}
		}
	}
	printf("\n");
#endif
	return 0;
}