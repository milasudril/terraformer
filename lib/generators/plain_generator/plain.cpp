//@	{"target": {"name": "./plain.o"}}

#include "./plain.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"

#include <stdexcept>

namespace
{
	// Need a polynomial whose
	// * Value is zero at x = 0 and x = 1 => P(x) = A(x)x(x - 1), for some other polynomial A
	// * Derivative is zero at x = 0 and x = 1
	// * The function value at x_0 \in ]0, 1[ is z_0
	//
	// P(x) is then a polynomial of degree 4, and A(x) = ax^2 + bx + c.
	// P^\prime(x) = A^\prime(x)x(x - 1) + A(x) d(x(x - 1))/dx. If x = 0 or x = 1, the first
	// term vanishes. Since
	//
	//  d(x(x - 1))/dx = 2x - 1
	//
	//  P^\prime(0) = -c  = 0 => c = 0
	//  P^\prime(1) = a + b  (since c = 0) => a = -b
	//
	//  Thus, A(x) = ax^2 - ax = ax(x - 1), which gives, P(x) = a(x(x - 1))^2
	//  P(x_0) =a(x_0(x_0 - 1))^2 = z_0 <=> a = z_0/((x_0(x_0 - 1))^2)
	//
	float midpoint_polynomial(float xi, float actual_elevation, terraformer::plain_edge_descriptor ped)
	{
		auto const denom = ped.xi_0*(ped.xi_0 - 1.0f);
		auto const a = (ped.elevation - actual_elevation)/(denom*denom);
		auto const p = xi*(xi - 1.0f);
		return a*p*p;
	}
}

void terraformer::replace_pixels(
	terraformer::span_2d<float> output,
	float pixel_size,
	terraformer::plain_descriptor const& params
)
{
	auto const w = output.width();
	auto const h = output.height();

	auto const w_float = static_cast<float>(w);
	auto const h_float = static_cast<float>(h);

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
		auto const eta = (static_cast<float>(y) + 0.5f)/h_float;
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const xi = (static_cast<float>(x) + 0.5f)/w_float;
			auto const x_interp_n_cubic = interp(nw_x, ne_x, xi);
			auto const x_interp_s_cubic = interp(sw_x, se_x, xi);

			auto const x_interp_n = x_interp_n_cubic + midpoint_polynomial(xi, x_interp_n_cubic, plain_edge_descriptor{
				.xi_0 = 0.5f,
				.elevation = params.n
			});
			auto const x_interp_s = x_interp_s_cubic + midpoint_polynomial(xi, x_interp_s_cubic, plain_edge_descriptor{
				.xi_0 = 0.5f,
				.elevation = params.s
			});

			auto const bicubic_val = interp(
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

			output(x, y) = bicubic_val;
		}
	}
}

terraformer::grayscale_image terraformer::generate(domain_size_descriptor const& size, plain_descriptor const& params)
{
	auto const size_factor = std::min(size.width, size.height);
	// Assume a bandwidth of at most 4 periods
	// Take 4 samples per period
	// Round up to next value that also contains a factor of 3, which is useful to have
	auto const min_pixel_count = 24.0f;
	auto const w_scaled = min_pixel_count*size.width/size_factor;
	auto const h_scaled = min_pixel_count*size.height/size_factor;
	auto const pixel_size = size_factor/min_pixel_count;

	grayscale_image ret{static_cast<uint32_t>(w_scaled + 0.5f), static_cast<uint32_t>(h_scaled + 0.5f)};
	replace_pixels(ret.pixels(), pixel_size, params);
	return ret;
}
