//@	{"target": {"name": "./plain.o"}}

#include "./plain.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/pixel_store/image.hpp"

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
						.ddx = bcd.ddx_0*bcd.x_m
					},
					terraformer::cubic_spline_control_point{
						.y = bcd.y_m,
						.ddx = bcd.ddx_m*bcd.x_m
					}
				),
				make_polynomial(
					terraformer::cubic_spline_control_point{
						.y = bcd.y_m,
						.ddx = bcd.ddx_m*(1.0f - bcd.x_m)
					},
					terraformer::cubic_spline_control_point{
						.y = bcd.y_1,
						.ddx = bcd.ddx_1*(1.0f - bcd.x_m)
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

	grayscale_image ret{
		static_cast<uint32_t>(w_scaled + 0.5f),
			static_cast<uint32_t>(h_scaled + 0.5f)
	};

	auto const w = ret.width();
	auto const h = ret.height();

	auto const w_float = static_cast<float>(w);
	auto const h_float = static_cast<float>(h);

	auto const west_to_east_north = boundary_curve(
		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.boundary.nw.elevation,
			.y_m = params.boundary.n.elevation,
			.y_1 = params.boundary.ne.elevation,
			.ddx_0 = params.boundary.nw.ddx*dom_size.width,
			.ddx_m = params.boundary.n.ddx*dom_size.width,
			.ddx_1 = params.boundary.ne.ddx*dom_size.width
		}
	);

	auto const west_to_east_south = boundary_curve(
		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.s,
			.y_0 = params.boundary.sw.elevation,
			.y_m = params.boundary.s.elevation,
			.y_1 = params.boundary.se.elevation,
			.ddx_0 = params.boundary.sw.ddx*dom_size.width,
			.ddx_m = params.boundary.s.ddx*dom_size.width,
			.ddx_1 = params.boundary.se.ddx*dom_size.width
		}
	);

	auto const north_to_south_west = boundary_curve(
		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.w,
			.y_0 = params.boundary.nw.elevation,
			.y_m = params.boundary.w.elevation,
			.y_1 = params.boundary.sw.elevation,
			.ddx_0 = params.boundary.nw.ddy*dom_size.height,
			.ddx_m = params.boundary.w.ddy*dom_size.height,
			.ddx_1 = params.boundary.sw.ddy*dom_size.height
		}
	);

	auto const north_to_south_east = boundary_curve{
		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.e,
			.y_0 = params.boundary.ne.elevation,
			.y_m = params.boundary.e.elevation,
			.y_1 = params.boundary.se.elevation,
			.ddx_0 = params.boundary.ne.ddy*dom_size.height,
			.ddx_m = params.boundary.e.ddy*dom_size.height,
			.ddx_1 = params.boundary.se.ddy*dom_size.height
		}
	};

	auto const z_m_interp_ns = make_polynomial(
		cubic_spline_control_point{
			.y = params.boundary.w.elevation,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.boundary.e.elevation,
			.ddx = 0.0f
		}
	);

	auto const z_m_interp_we = make_polynomial(
		cubic_spline_control_point{
			.y = params.boundary.n.elevation,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.boundary.s.elevation,
			.ddx = 0.0f
		}
	);

	auto const y_m = make_polynomial(
		cubic_spline_control_point{
			.y = params.edge_midpoints.w,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.edge_midpoints.e,
			.ddx = 0.0f
		}
	);

	auto const x_m = make_polynomial(
		cubic_spline_control_point{
			.y = params.edge_midpoints.n,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.edge_midpoints.s,
			.ddx = 0.0f
		}
	);

	auto const ddy_0 = boundary_curve{		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.boundary.nw.ddy*dom_size.height,
			.y_m = 2.0f*params.boundary.n.ddy*dom_size.height,
			.y_1 = params.boundary.ne.ddy*dom_size.height,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const ddy_m = make_polynomial(
		cubic_spline_control_point{
			.y = params.boundary.w.ddy*dom_size.height,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.boundary.e.ddy*dom_size.height,
			.ddx = 0.0f
		}
	);

	auto const ddy_1 = boundary_curve{		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.boundary.sw.ddy*dom_size.height,
			.y_m = 2.0f*params.boundary.s.ddy*dom_size.height,
			.y_1 = params.boundary.se.ddy*dom_size.height,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const ddx_0 = boundary_curve{		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.boundary.nw.ddx*dom_size.width,
			.y_m = 2.0f*params.boundary.w.ddx*dom_size.width,
			.y_1 = params.boundary.sw.ddx*dom_size.width,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const ddx_m = make_polynomial(
		cubic_spline_control_point{
			.y = params.boundary.n.ddx*dom_size.width,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.boundary.s.ddx*dom_size.width,
			.ddx = 0.0f
		}
	);

	auto const ddx_1 = boundary_curve{		boundary_curve_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.boundary.ne.ddx*dom_size.width,
			.y_m = 2.0f*params.boundary.e.ddx*dom_size.width,
			.y_1 = params.boundary.se.ddx*dom_size.width,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

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

			auto const north_to_south = boundary_curve(
				boundary_curve_descriptor{
				.x_m = bounded_value<open_open_interval{0.0f, 1.0f}, 0.5f>{y_m(xi)},
				.y_0 = west_to_east_north(xi),
				.y_m = z_m_interp_ns(xi),
				.y_1 = west_to_east_south(xi),
				.ddx_0 = ddy_0(xi),
				.ddx_m = ddy_m(xi),
				.ddx_1 = ddy_1(xi)
				}
			);

			auto const west_to_east = boundary_curve(
				boundary_curve_descriptor{
				.x_m = bounded_value<open_open_interval{0.0f, 1.0f}, 0.5f>{x_m(eta)},
				.y_0 = north_to_south_west(eta),
				.y_m = z_m_interp_we(eta),
				.y_1 = north_to_south_east(eta),
				.ddx_0 = ddx_0(eta),
				.ddx_m = ddx_m(eta),
				.ddx_1 = ddx_1(eta)
				}
			);

			ret(x, y) = 0.5f*(north_to_south(eta) + west_to_east(xi));
		}
	}

	return ret;
}

