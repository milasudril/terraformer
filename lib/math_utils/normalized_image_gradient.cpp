//@	{"target":{"name":"normalized_image_gradient.o"}}

#include "./normalized_image_gradient.hpp"
#include "lib/common/span_2d.hpp"
#include <cmath>

void terraformer::compute_normalized_gradient(
	scanline_processing_job_info const& jobinfo,
	span_2d<direction> output,
	span_2d<float const> input
)
{
	auto const input_y_offset = jobinfo.input_y_offset;
	auto const w = static_cast<int32_t>(output.width());
	auto const h = static_cast<int32_t>(output.height());
	using clamp_tag = span_2d_extents::clamp_tag;

	for(int32_t y = 0; y != h; ++y)
	{
		for(int32_t x = 0; x != w; ++x)
		{
			auto const y_in = y + input_y_offset;
			auto const dx = input(x + 1, y_in, clamp_tag{}) - input(x - 1, y_in, clamp_tag{});
			auto const dy = input(x, y_in + 1, clamp_tag{}) - input(x, y_in - 1, clamp_tag{});
			output(x, y) = direction{displacement{dx, dy, 0.0f}};
		}
	}
}