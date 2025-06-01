//@	{"target": {"name": "./plain.o"}}

#include "./plain.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"

#include <stdexcept>

namespace
{
	struct boundary_curve_descriptor
	{
		terraformer::bounded_value<terraformer::open_open_interval{0.0f, 1.0f}, 0.5f> x_m{0.5f};

		float y_0 = 0.0f;
		float y_m = 0.0f;
		float y_1 = 0.0f;
		float ddx_0 = 0.0f;
		float ddx_m = 0.0f;
		float ddx_1 = 0.0f;
	};

	class boundary_curve
	{
	public:
		constexpr explicit boundary_curve(boundary_curve_descriptor const& bcd):
			m_poly{
				make_polynomial(
					terraformer::cubic_spline_control_point{
						.y = bcd.y_0,
						.ddx = 0.0f
					},
					terraformer::cubic_spline_control_point{
						.y = bcd.y_m,
						.ddx = 0.0f
					}
				),
				make_polynomial(
					terraformer::cubic_spline_control_point{
						.y = bcd.y_m,
						.ddx = 0.0f
					},
					terraformer::cubic_spline_control_point{
						.y = bcd.y_1,
						.ddx = 0.0f
					}
				)
			},
			m_xm{bcd.x_m}
		{
		}

		constexpr float operator()(float x) const
		{
			return (x < m_xm)? m_poly[0](x/m_xm) : m_poly[1]((x - m_xm)/(1.0f - m_xm));
		}

	private:
		std::array<terraformer::polynomial<3>, 2> m_poly;
		terraformer::bounded_value<terraformer::open_open_interval{0.0f, 1.0f}, 0.5f> m_xm;
	};
}

terraformer::grayscale_image terraformer::generate(
	domain_size_descriptor const& dom_size,
	plain_descriptor const& params
)
{
	auto const size_factor = std::min(dom_size.width, dom_size.height);
	// Assume a bandwidth of at most 4 periods
	// Take 4 samples per period
	// Round up to next value that also contains a factor of 3, which is useful to have
	auto const min_pixel_count = 24.0f;
	auto const w_scaled = min_pixel_count*dom_size.width/size_factor;
	auto const h_scaled = min_pixel_count*dom_size.height/size_factor;

	grayscale_image ret{static_cast<uint32_t>(w_scaled + 0.5f), static_cast<uint32_t>(h_scaled + 0.5f)};

	auto const w = ret.width();
	auto const h = ret.height();

	auto const w_float = static_cast<float>(w);
	auto const h_float = static_cast<float>(h);

	auto const west_to_east_north = boundary_curve(
		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.elevations.nw,
			.y_m = params.elevations.n,
			.y_1 = params.elevations.ne,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	);

	auto const west_to_east_south = boundary_curve(
		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.s,
			.y_0 = params.elevations.sw,
			.y_m = params.elevations.s,
			.y_1 = params.elevations.se,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	);

	auto const north_to_south_west = boundary_curve(
		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.w,
			.y_0 = params.elevations.nw,
			.y_m = params.elevations.w,
			.y_1 = params.elevations.sw,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	);

	auto const north_to_south_east = boundary_curve(
		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.e,
			.y_0 = params.elevations.ne,
			.y_m = params.elevations.e,
			.y_1 = params.elevations.se,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	);

	auto const y_m_interp_ns = make_polynomial(
		cubic_spline_control_point{
			.y = params.elevations.w,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.elevations.e,
			.ddx = 0.0f
		}
	);

	auto const y_m_interp_we = make_polynomial(
		cubic_spline_control_point{
			.y = params.elevations.n,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.elevations.s,
			.ddx = 0.0f
		}
	);

	auto const cos_theta = std::cos(2.0f*std::numbers::pi_v<float>*params.orientation);
	auto const sin_theta = std::sin(2.0f*std::numbers::pi_v<float>*params.orientation);

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const eta_in = (static_cast<float>(y) + 0.5f)/h_float - 0.5f;
			auto const xi_in = (static_cast<float>(x) + 0.5f)/w_float - 0.5f;

			auto const xi  =  xi_in*cos_theta + eta_in*sin_theta + 0.5f;
			auto const eta = -xi_in*sin_theta + eta_in*cos_theta + 0.5f;

			auto const x_interp_n = west_to_east_north(xi);
			auto const x_interp_s = west_to_east_south(xi);
			auto const y_interp_w = north_to_south_west(eta);
			auto const y_interp_e = north_to_south_east(eta);

			auto const north_to_south = boundary_curve(
				boundary_curve_descriptor{
				.x_m = lerp(params.edge_midpoints.w, params.edge_midpoints.e, xi),
				.y_0 = x_interp_n,
				.y_m = y_m_interp_ns(xi),
				.y_1 = x_interp_s,
				.ddx_0 = 0.0f,
				.ddx_m = 0.0f,
				.ddx_1 = 0.0f
				}
			);

			auto const west_to_east = boundary_curve(
				boundary_curve_descriptor{
				.x_m = lerp(params.edge_midpoints.n, params.edge_midpoints.s, eta),
				.y_0 = y_interp_w,
				.y_m = y_m_interp_we(eta),
				.y_1 = y_interp_e,
				.ddx_0 = 0.0f,
				.ddx_m = 0.0f,
				.ddx_1 = 0.0f
				}
			);

			ret(x, y) = 0.5f*(north_to_south(eta) + west_to_east(xi));
		}
	}

	return ret;
}

