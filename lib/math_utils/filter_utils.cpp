//@	{"target":{"name":"filter_utils.o"}}

#include "./filter_utils.hpp"

#include <cassert>

void terraformer::make_filter_input(
	span_2d<std::complex<float>> output,
	span_2d<float const> input,
	uint32_t y_input_offset
)
{
	auto const w = output.width();
	auto const h = output.height();
	assert(w%2 == 0);
	assert(h%2 == 0);

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

terraformer::basic_image<std::complex<float>> terraformer::make_filter_input(span_2d<float const> input)
{
	auto const w = input.width();
	auto const h = input.height();
	assert(w%2 == 0);
	assert(h%2 == 0);

	basic_image<std::complex<float>> filter_input{w, h};

	make_filter_input(filter_input, input, 0);

	return filter_input;
}