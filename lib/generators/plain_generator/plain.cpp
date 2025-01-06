//@	{"target": {"name": "./plain.o"}}

#include "./plain.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"

#include <stdexcept>

void terraformer::replace_pixels(
	terraformer::span_2d<float> output,
	float pixel_size,
	terraformer::plain_descriptor const& params
)
{
	auto const w = output.width();
	auto const h = output.height();
	if(w < 2u || h < 2u)
	{ throw std::runtime_error{"Domain size must be at least 2 x 2 pixels"}; }

	auto const w_float = static_cast<float>(w - 1);
	auto const h_float = static_cast<float>(h - 1);

	cubic_spline_control_point const nw_x{
		.y = params.nw.elevation,
		.ddx = pixel_size*w_float*params.nw.ddx
	};

	cubic_spline_control_point const ne_x{
		.y = params.ne.elevation,
		.ddx = pixel_size*w_float*params.ne.ddx
	};

	cubic_spline_control_point const sw_x{
		.y = params.sw.elevation,
		.ddx = pixel_size*w_float*params.sw.ddx
	};

	cubic_spline_control_point const se_x{
		.y = params.se.elevation,
		.ddx = pixel_size*w_float*params.se.ddx
	};

	auto const nw_ddy = pixel_size*h_float*params.nw.ddy;
	auto const ne_ddy = pixel_size*h_float*params.ne.ddy;
	auto const sw_ddy = pixel_size*h_float*params.sw.ddy;
	auto const se_ddy = pixel_size*h_float*params.se.ddy;

	for(uint32_t y = 0; y != h; ++y)
	{
		auto const eta = static_cast<float>(y)/h_float;
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const xi = static_cast<float>(x)/w_float;

			auto const x_interp_n = interp(nw_x, ne_x, xi);
			auto const x_interp_s = interp(sw_x, se_x, xi);

			output(x, y) = interp(
				cubic_spline_control_point{
					.y = x_interp_n,
					.ddx = std::lerp(nw_ddy, ne_ddy, xi)
				},
				cubic_spline_control_point{
					.y = x_interp_s,
					.ddx = std::lerp(sw_ddy, se_ddy, xi)
				},
				eta
			);
		}
	}
}