//@	{"target":{"name":"./butter_lp_2d.o"}}

#include "./butter_lp_2d.hpp"
#include "./dft_engine.hpp"

#include <cassert>

terraformer::grayscale_image terraformer::apply(
	butter_lp_2d_descriptor const& params,
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

	{
		auto const f_x = params.f_x;
		auto const f_y = params.f_y;
		auto const w_float = static_cast<float>(w);
		auto const h_float = static_cast<float>(h);
		auto const x_0 = 0.5f*w_float;
		auto const x_y = 0.5f*h_float;
		auto const cos_theta = std::cos(params.y_direction);
		auto const sin_theta = std::sin(params.y_direction);
		auto const hf_rolloff = params.hf_rolloff;
		for(uint32_t y = 0; y != h; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{
				auto const xi_in = static_cast<float>(x) - x_0;
				auto const eta_in = static_cast<float>(y) - x_y;

				auto const xi = (xi_in*cos_theta + eta_in*sin_theta)/f_x;
				auto const eta = (-xi_in*sin_theta + eta_in*cos_theta)/f_y;

				auto const r2 = xi*xi + eta*eta;

				auto const lpf = 1.0f/std::sqrt(1.0f + std::pow(r2, hf_rolloff));
				transformed_input(x, y) *= lpf;
			}
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
