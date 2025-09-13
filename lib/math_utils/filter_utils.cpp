//@	{"target":{"name":"filter_utils.o"}}

#include "./filter_utils.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/utils.hpp"
#include "lib/execution/signaling_counter.hpp"
#include "lib/common/chunk_by_chunk_count_view.hpp"

#include <cassert>
#include <ranges>

void terraformer::make_filter_input(
	span_2d<float const> input,
	uint32_t y_input_offset,
	span_2d<std::complex<float>> output
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
