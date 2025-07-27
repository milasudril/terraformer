//@	{"target":{"name":"./butter_bp_2d.o"}}

#include "./butter_bp_2d.hpp"
#include "./dft_engine.hpp"

#include <cassert>

terraformer::grayscale_image terraformer::apply(
	butter_bp_2d_descriptor const&,
	span_2d<float const> input
)
{
	auto const w = input.width();
	auto const h = input.height();

	assert(w%2 == 0);
	assert(h%2 == 0);

	basic_image<std::complex<float>> filter_input{w, h};
	{
		auto sign_y = 1.0f;
		for(uint32_t y = 0; y != h; ++y)
		{
			auto sign_x = 1.0f;
			for(uint32_t x = 0; x != w; ++x)
			{
				filter_input(x, y) = input(x, y) * sign_y * sign_x;
				sign_x *= -1.0f;
			}
			sign_y *= -1.0f;
		}
	}

	dft_execution_plan plan_forward{
		span_2d_extents{
			.width = w,
			.height = h
		},
		dft_direction::forward
	};
	terraformer::basic_image<std::complex<float>> transformed_input{w, h};
	plan_forward.execute(std::as_const(filter_input).pixels().data(), transformed_input.pixels().data());

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const filter_value = 1.0f;
			transformed_input(x, y) *= filter_value;
		}
	}

	dft_execution_plan plan_backward{
		span_2d_extents{
			.width = w,
			.height = h
		},
		dft_direction::backward
	};
	plan_backward.execute(std::as_const(transformed_input).pixels().data(), filter_input.pixels().data());

	grayscale_image filtered_output{w, h};
	{
		auto sign_y = 1.0f;
		for(uint32_t y = 0; y < h; ++y)
		{
			auto sign_x = 1.0f;
			for(uint32_t x = 0; x < w; ++x)
			{
				filtered_output(x, y) = filter_input(x, y).real() * sign_x * sign_y;
				sign_x *= -1.0f;
			}
			sign_y *= -1.0f;
		}
	}

	return filtered_output;
}
