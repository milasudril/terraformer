//@	{"target": {"name":"./bump_field.o"}}

#include "./bump_field.hpp"

#include <limits>

std::ranges::min_max_result<float> terraformer::generate(span_2d<float> output_buffer, bump_field const& bumps)
{
	std::ranges::min_max_result ret{
		.min =  std::numeric_limits<float>::infinity(),
		.max = -std::numeric_limits<float>::infinity()
	};
	for(uint32_t y = 0; y != output_buffer.height(); ++y)
	{
		for(uint32_t x = 0; x != output_buffer.width(); ++x)
		{
			auto convsum = bumps(x, y);
			ret.min = std::min(convsum, ret.min);
			ret.max = std::max(convsum, ret.max);
			output_buffer(x, y) = convsum;
		}
	}
	return ret;
}
