//@	{"target":{"name":"filter_utils.o"}}

#include "./filter_utils.hpp"
#include "lib/execution/signaling_counter.hpp"
#include "lib/common/chunk_by_chunk_count_view.hpp"

#include <cassert>
#include <ranges>

void terraformer::make_filter_input(
	span_2d<std::complex<float>> output,
	span_2d<float const> input,
	uint32_t y_input_offset
)
{
	auto const w = output.width();
	auto const h = output.height();

	auto sign_y = (y_input_offset%2 == 0)? 1.0f : 0.0f;
	for(uint32_t y = 0; y != h; ++y)
	{
		auto sign_x = 1.0f;
		for(uint32_t x = 0; x != w; ++x)
		{
			output(x, y) = input(x, y + y_input_offset) * sign_y * sign_x;
			sign_x *= -1.0f;
		}
		sign_y *= -1.0f;
	}
}

terraformer::signaling_counter terraformer::make_filter_input(
	span_2d<std::complex<float>> output,
	span_2d<float const> input,
	thread_pool<move_only_function<void()>>& workers
)
{
	auto const w = input.width();
	auto const h = input.height();
	assert(w%2 == 0);
	assert(h%2 == 0);
	assert(w == output.width());
	assert(h == output.height());

	auto const n_workers = workers.max_concurrency();
	signaling_counter ret{n_workers};

	for(auto chunk: chunk_by_chunk_count_view{std::ranges::iota_view{0u, input.height()}, n_workers})
	{
		workers.submit(
			[
				output = output.scanlines(
					scanline_range{
						.begin = chunk.front(),
						.end = chunk.back() + 1
					}
				),
				input,
				y_offset = chunk.front(),
				&counter = ret.get_state()
			](){
				make_filter_input(output, input, y_offset);
				counter.decrement();
			}
		);
	}

	return ret;
}