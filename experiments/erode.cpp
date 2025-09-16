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
#include <cstring>
#include <random>
#include <bit>

using thread_pool_type = terraformer::thread_pool<terraformer::move_only_function<void()>>;

void erode(
	terraformer::scanline_processing_job_info const& jobinfo,
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	terraformer::span_2d<float const> noise,
	terraformer::span<terraformer::random_generator> rngs
)
{
//	using clamp_tag = terraformer::span_2d_extents::clamp_tag;
	auto const input_y_offset = jobinfo.input_y_offset;
	std::uniform_real_distribution spawn{0.0f, 1.0f};
	auto& rng = rngs[rngs.element_indices().front() + thread_pool_type::current_worker()];
	for(int32_t y = 0; y != static_cast<int32_t>(output.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(output.width()); ++x)
		{
			auto const noise_val = noise(x , y + input_y_offset)*input(x, y + input_y_offset)/3500.0f;
			if(spawn(rng) < 2.0f*15.0f*noise_val/4096.0f)
			{ output(x, y) = 1.0f; }
			else
			{ output(x, y) = 0.0f; }
		}
	}
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

[[nodiscard]] terraformer::batch_result<void> make_white_noise(
	terraformer::span_2d<float> output,
	thread_pool_type& workers,
	terraformer::span<terraformer::random_generator> rngs
)
{
	assert(workers.max_concurrency() == std::size(rngs).get());

	return process_scanlines(
		output,
		workers,
		[](
			terraformer::scanline_processing_job_info const&,
			terraformer::span_2d<float> output,
			terraformer::span<terraformer::random_generator> rngs
		){
			auto const index = rngs.element_indices().front() + thread_pool_type::current_worker();
			make_white_noise(output, rngs[index]);
		},
		rngs
	);
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

	auto white_noise_buffer = buffer_a;
	auto filtered_noise_buffer_a = buffer_a;
	auto filtered_noise_buffer_b = buffer_a;
	auto accumulated_noise = buffer_a;
	auto filter_mask = buffer_a;

	auto input = buffer_a.pixels();
	auto output = buffer_b.pixels();
	auto noise_input = filtered_noise_buffer_a.pixels();
	auto noise_output = filtered_noise_buffer_b.pixels();

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

	auto pending_filter_mask = process_scanlines(
		filter_mask.pixels(),
		comp_ctxt.workers,
		[]<class ... Args>(Args&&... params){
			make_filter_mask(std::forward<Args>(params)...);
		},
		terraformer::butter_lp_2d_descriptor{
			.f_x = 49152.0f/4096.0f,
			.f_y = 49152.0f/4096.0f,
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
		noise_output,
		comp_ctxt,
		filter_mask.pixels()
	).wait();

	terraformer::process_scanlines(
		noise_output,
		comp_ctxt.workers,
		[](
			auto const&,
			terraformer::span_2d<float> output,
			std::ranges::minmax_result<float> input_range
		){
			return terraformer::normalize(output, input_range);
		},
		terraformer::fold(
			noise_output,
			comp_ctxt.workers,
			[](auto const&, terraformer::span_2d<float const> output){
				return minmax_value(output);
			}
		).get_result(fold_minmax_value)
	).wait();

	std::swap(noise_input, noise_output);

	for(size_t k = 0 ; k != 1; ++k)
	{
		auto next_result = process_scanlines(
			output,
			comp_ctxt.workers,
			[]<class ... Args>(Args&&... args) {
				erode(std::forward<Args>(args)...);
			},
			std::as_const(input),
			std::as_const(noise_input),
			rngs
		);

		make_white_noise(white_noise_buffer.pixels(), comp_ctxt.workers, rngs).wait();
		terraformer::apply_filter(
			white_noise_buffer.pixels(),
			noise_output,
			comp_ctxt,
			filter_mask.pixels()
		).wait();

		process_scanlines(
			noise_output,
			comp_ctxt.workers,
			[](
				auto const&,
				terraformer::span_2d<float> output,
				std::ranges::minmax_result<float> input_range
			){
				return terraformer::normalize(output, input_range);
			},
			terraformer::fold(
				noise_output,
				comp_ctxt.workers,
				[](auto const&, terraformer::span_2d<float const> output){
					return minmax_value(output);
				}
			).get_result(fold_minmax_value)
		).wait();

		next_result.wait();
		std::swap(noise_input, noise_output);
		std::swap(input, output);
		printf("\r%zu   ", k);
		fflush(stdout);
	}

	store(input, "/dev/shm/slask.exr");

	return 0;
}