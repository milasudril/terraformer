//@	{"target": {"name": "./plain.o"}}

#include "./plain.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/math_utils/quintic_polynomial.hpp"

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
	float edge_offset(float xi, float actual_elevation, terraformer::plain_edge_descriptor ped)
	{
		auto const denom = ped.xi_0*(ped.xi_0 - 1.0f);
		auto const a = (ped.elevation - actual_elevation)/(denom*denom);
		auto const p = xi*(xi - 1.0f);
		return a*p*p;
	}

	// P(x) = Ax(x - 1)
	// P(x_0) = z_0
	// A = z_0/(x_0(x_0 - 1))
	float center_offset(float xi, float actual_elevation, float desired_elevation)
	{
		auto const xi_0 = 0.5f;
		auto const denom = xi_0*(xi_0 - 1.0f);
		auto const a = (desired_elevation - actual_elevation)/denom;
		auto const p = xi*(xi - 1.0f);
		return a*p;
	}
}

terraformer::grayscale_image terraformer::generate(
	domain_size_descriptor const& dom_size,
	plain_descriptor_new const& params
)
{
	using xm_type = bounded_value<open_open_interval{0.0f, 1.0f}, 0.5f>;

	auto const size_factor = std::min(dom_size.width, dom_size.height);
	// Assume a bandwidth of at most 4 periods
	// Take 4 samples per period
	// Round up to next value that also contains a factor of 3, which is useful to have
	auto const min_pixel_count = 24.0f;
	auto const w_scaled = min_pixel_count*dom_size.width/size_factor;
	auto const h_scaled = min_pixel_count*dom_size.height/size_factor;

	//auto const pixel_size = size_factor/min_pixel_count;

	grayscale_image ret{static_cast<uint32_t>(w_scaled + 0.5f), static_cast<uint32_t>(h_scaled + 0.5f)};

	auto const w = ret.width();
	auto const h = ret.height();

	auto const w_float = static_cast<float>(w);
	auto const h_float = static_cast<float>(h);

	auto const west_to_east_north = make_polynomial(
		quintic_polynomial_descriptor{
			.x_m = params.midpoints.n,
			.y_0 = params.elevations.nw,
			.y_m = params.elevations.n,
			.y_1 = params.elevations.ne,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	);

	auto const west_to_east_south = make_polynomial(
		quintic_polynomial_descriptor{
			.x_m = params.midpoints.s,
			.y_0 = params.elevations.sw,
			.y_m = params.elevations.s,
			.y_1 = params.elevations.se,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	);

	auto const west_to_east_interior = make_polynomial(
		quintic_polynomial_descriptor{
			.x_m = params.midpoints.c_we,
			.y_0 = params.elevations.w,
			.y_m = params.elevations.c,
			.y_1 = params.elevations.e,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	);

	auto const ns_midpoint_location = make_polynomial(
		cubic_polynomial_with_crit_point_descriptor{
		//	.x_crit = params.midpoints.c_we,
			.y_0 = params.midpoints.w,
			.y_crit = params.midpoints.c_ns,
			.y_1 = params.midpoints.e
		}
	);

	for(uint32_t y = 0; y != h; ++y)
	{
		auto const eta = (static_cast<float>(y) + 0.5f)/h_float;
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const xi = (static_cast<float>(x) + 0.5f)/w_float;

			auto const x_interp_n = west_to_east_north(xi);
			auto const x_interp_s = west_to_east_south(xi);

			auto const north_to_south = make_polynomial(
				quintic_polynomial_descriptor{
				.x_m = xm_type{ns_midpoint_location(xi)},
			//	.x_m = lerp(params.midpoints.w, params.midpoints.e, xi),
				.y_0 = x_interp_n,
				.y_m = west_to_east_interior(xi),
				.y_1 = x_interp_s,
				.ddx_0 = 0.0f,
				.ddx_m = 0.0f,
				.ddx_1 = 0.0f
				}
			);

			ret(x, y) = north_to_south(eta);
		}
	}

	return ret;
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

			auto const x_interp_n = x_interp_n_cubic + edge_offset(xi, x_interp_n_cubic, plain_edge_descriptor{
				.xi_0 = 0.5f,
				.elevation = params.n
			});
			auto const x_interp_s = x_interp_s_cubic + edge_offset(xi, x_interp_s_cubic, plain_edge_descriptor{
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

			auto const elev_lerp = std::lerp(params.w, params.e , xi);

			output(x, y) = bicubic_val + edge_offset(eta, bicubic_val, plain_edge_descriptor{
				.xi_0 = 0.5f,
				.elevation = elev_lerp + center_offset(xi, elev_lerp, params.midpoint)
			});
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
