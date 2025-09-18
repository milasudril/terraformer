//@	{"target":{"name":"erode.o"}}

#include "lib/common/move_only_function.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/utils.hpp"
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
#include "lib/generators/domain/domain_size.hpp"

#include <algorithm>
#include <cstring>
#include <random>
#include <bit>

using thread_pool_type = terraformer::thread_pool<terraformer::move_only_function<void()>>;

struct stream_spawn_descriptor
{
	float stream_distance;
};

struct stream_point
{
	terraformer::location where;
	terraformer::displacement gradient;
};

struct stream
{
	float total_flow;
	terraformer::single_array<stream_point> points;
};

template<class Rng>
struct counting_rng :public Rng
{
	using Rng::Rng;
	counting_rng(counting_rng const&) = delete;
	counting_rng(counting_rng&&) = default;

	auto operator()()
	{
		++count;
		return Rng::operator()();
	}

	size_t count{0};
};

stream make_stream(terraformer::span_2d<float const> heightmap, stream_point start_at, float dx, float dy)
{
	stream ret;
	ret.total_flow = 1.0f;
	ret.points.push_back(start_at);
	auto const max_length =
		static_cast<size_t>(
			std::sqrt(static_cast<float>(heightmap.width())*static_cast<float>(heightmap.height()))
		);

	while(true)
	{
		auto const grad_norm = norm(start_at.gradient);
		if(grad_norm < 1.0f/32768.0f || std::size(ret.points).get() == max_length)
		{ return ret; }

		start_at.where += -1.0f*terraformer::direction{start_at.gradient};

		auto const x = start_at.where[0];
		auto const y = start_at.where[1];
		auto const dz_dx = (
				interp(heightmap, x + 1.0f, y, terraformer::clamp_at_boundary{})
			- interp(heightmap, x - 1.0f, y, terraformer::clamp_at_boundary{})
		)/(2.0f*dx);
		auto const dz_dy = (
				interp(heightmap, x, y + 1.0f, terraformer::clamp_at_boundary{})
			- interp(heightmap, x, y - 1.0f, terraformer::clamp_at_boundary{})
		)/(2.0f*dy);
		start_at.gradient = terraformer::displacement{dz_dx, dz_dy, 0.0f};

		ret.points.push_back(start_at);
	}
}

terraformer::single_array<stream> generate_streams(
	terraformer::scanline_processing_job_info const& jobinfo,
	// TODO: I am not going to write to output, but it must be here to partition the image
	terraformer::span_2d_extents extents,
	terraformer::domain_size_descriptor dom_size,
	stream_spawn_descriptor const& params,
	terraformer::span_2d<float const> heightmap,
	terraformer::span_2d<float const> noise,
	terraformer::span<counting_rng<terraformer::random_generator>> rngs
)
{
	using clamp_tag = terraformer::span_2d_extents::clamp_tag;
	auto const w = extents.width;
	auto const h = extents.height;
	auto const total_height = heightmap.height();
	auto const input_y_offset = static_cast<int32_t>(jobinfo.input_y_offset);
	auto const dx = dom_size.width/static_cast<float>(w);
	auto const dy = dom_size.height/static_cast<float>(total_height);
	auto const pixel_size = dx*dy;
	auto const stream_density = pixel_size/(params.stream_distance*params.stream_distance);

	std::uniform_real_distribution spawn{0.0f, 1.0f};
	terraformer::single_array<stream> ret;
	auto& rng = rngs[rngs.element_indices().front() + thread_pool_type::current_worker()];

	for(int32_t y = 0; y != static_cast<int32_t>(h); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(w); ++x)
		{
			auto const dz_dx = (
				  heightmap(x + 1, y + input_y_offset, clamp_tag{})
				- heightmap(x - 1, y + input_y_offset, clamp_tag{})
			)/(2.0f*dx);

			auto const dz_dy = (
				  heightmap(x, y + input_y_offset + 1, clamp_tag{})
				- heightmap(x, y + input_y_offset - 1, clamp_tag{})
			)/(2.0f*dy);

			auto const noise_val = noise(x , y + input_y_offset);
			if(spawn(rng) < noise_val*stream_density && (dz_dx != 0.0f || dz_dy != 0.0f))
			{
				ret.push_back(
					make_stream(
						heightmap,
						stream_point{
							.where = terraformer::location{
								static_cast<float>(x),
								static_cast<float>(y),
								0.0f,
							},
							.gradient = terraformer::displacement{dz_dx, dz_dy, 0.0f}
						},
						dx,
						dy
					)
				);
			}
		}
	}

	return ret;
}

void make_white_noise(terraformer::span_2d<float> output, counting_rng<terraformer::random_generator>& rng)
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
	terraformer::span<counting_rng<terraformer::random_generator>> rngs
)
{
	assert(workers.max_concurrency() == std::size(rngs).get());

	return process_scanlines(
		output,
		workers,
		[](
			terraformer::scanline_processing_job_info const&,
			terraformer::span_2d<float> output,
			terraformer::span<counting_rng<terraformer::random_generator>> rngs
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
			n_threads
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
	terraformer::random_generator master_rng{terraformer::rng_seed_type{}};
	terraformer::single_array<counting_rng<terraformer::random_generator>> rngs;
	for(size_t k = 0; k != n_threads; ++k)
	{
		rngs.push_back(counting_rng<terraformer::random_generator>{terraformer::generate_rng_seed(master_rng)});
	}

	auto pending_noise = make_white_noise(white_noise_buffer.pixels(), comp_ctxt.workers, rngs);

	pending_filter_mask.wait();
	pending_noise.wait();

	apply_filter(
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
		process_scanlines(
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
		puts("Generating streams");
		auto next_result = process_scanlines(
			output.extents(),
			comp_ctxt.workers,
			[]<class ... Args>(Args&&... args) {
				return generate_streams(std::forward<Args>(args)...);
			},
			terraformer::domain_size_descriptor{
				.width = 49152.0f,
				.height = 49152.0f
			},
			stream_spawn_descriptor{
				.stream_distance = 512.0f
			},
			std::as_const(input),
			std::as_const(noise_input),
			terraformer::span{std::begin(rngs), std::end(rngs)}
		);
#if 0
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
			process_scanlines(
				noise_output,
				comp_ctxt.workers,
				[](auto const&, terraformer::span_2d<float const> output){
					return minmax_value(output);
				}
			).get_result(fold_minmax_value)
		).wait();
#endif
		puts("Folding result");
		auto const res = next_result.get_result(
			[](auto&& streams){
				terraformer::single_array<stream> ret{};
				for(auto& item: std::ranges::join_view{streams})
				{ ret.push_back(std::move(item)); }
#if 0
				for(auto& outer : streams)
				{
					for(auto& inner : outer)
					{ ret.push_back(std::move(inner)); }
				}
#endif
				return ret;
			}
		);

		printf("Number of streams: %zu\n", std::size(res).get());
		std::swap(noise_input, noise_output);
		std::swap(input, output);
		printf("\r%zu   ", k);
		fflush(stdout);
	}

	store(input, "/dev/shm/slask.exr");

	return 0;
}