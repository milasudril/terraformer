//@	{"target": {"name": "./plain.o"}}

#include "./plain.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"

#include <stdexcept>

namespace
{
	struct boundary_curve_interp_descriptor
	{
		terraformer::bounded_value<terraformer::open_open_interval{0.0f, 1.0f}, 0.5f> x_m{0.5f};

		float y_0 = 0.0f;
		float y_m = 0.0f;
		float y_1 = 0.0f;
	};

	class boundary_curve_interp
	{
	public:
		constexpr explicit boundary_curve_interp(boundary_curve_interp_descriptor const& bcd):
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

	struct boundary_curve_grad_descriptor
	{
		terraformer::bounded_value<terraformer::open_open_interval{0.0f, 1.0f}, 0.5f> x_m{0.5f};

		float ddx_0 = 0.0f;
		float ddx_m = 0.0f;
		float ddx_1 = 0.0f;
	};

	class boundary_curve_grad
	{
	public:
		constexpr explicit boundary_curve_grad(boundary_curve_grad_descriptor const& bcd):
			m_poly{
				make_polynomial(
					terraformer::cubic_spline_control_point{
						.y = 0.0f,
						.ddx = bcd.ddx_0*bcd.x_m
					},
					terraformer::cubic_spline_control_point{
						.y = 0.0f,
						.ddx = bcd.ddx_m*bcd.x_m
					}
				),
				make_polynomial(
					terraformer::cubic_spline_control_point{
						.y = 0.0f,
						.ddx = bcd.ddx_m*(1.0f - bcd.x_m)
					},
					terraformer::cubic_spline_control_point{
						.y = 0.0f,
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

terraformer::plain terraformer::generate(
	domain_size_descriptor const& dom_size,
	plain_descriptor const& params
)
{
	auto const size_factor = std::min(dom_size.width, dom_size.height);

	auto const min_pixel_count = 48.0f;
	auto const w_scaled = min_pixel_count*dom_size.width/size_factor;
	auto const h_scaled = min_pixel_count*dom_size.height/size_factor;

	plain ret{
		.z_interp = grayscale_image{
			static_cast<uint32_t>(w_scaled + 0.5f),
			static_cast<uint32_t>(h_scaled + 0.5f)
		},
		.z_grad = grayscale_image{
			static_cast<uint32_t>(w_scaled + 0.5f),
			static_cast<uint32_t>(h_scaled + 0.5f)
		}
	};

	auto const w = ret.z_interp.width();
	auto const h = ret.z_interp.height();

	auto const w_float = static_cast<float>(w);
	auto const h_float = static_cast<float>(h);

	//auto const pixel_size_x = dom_size.width/(w_float - 1.0f);
	auto const ddy_scale = dom_size.height;

	auto const west_to_east_north_interp = boundary_curve_interp(
		boundary_curve_interp_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.boundary.nw.elevation,
			.y_m = params.boundary.n.elevation,
			.y_1 = params.boundary.ne.elevation
		}
	);

	auto const west_to_east_south_interp = boundary_curve_interp(
		boundary_curve_interp_descriptor{
			.x_m = params.edge_midpoints.s,
			.y_0 = params.boundary.sw.elevation,
			.y_m = params.boundary.s.elevation,
			.y_1 = params.boundary.se.elevation
		}
	);

	auto const north_to_south_west_interp = boundary_curve_interp(
		boundary_curve_interp_descriptor{
			.x_m = params.edge_midpoints.w,
			.y_0 = params.boundary.nw.elevation,
			.y_m = params.boundary.w.elevation,
			.y_1 = params.boundary.sw.elevation
		}
	);

	auto const north_to_south_east_interp = boundary_curve_interp(
		boundary_curve_interp_descriptor{
			.x_m = params.edge_midpoints.e,
			.y_0 = params.boundary.ne.elevation,
			.y_m = params.boundary.e.elevation,
			.y_1 = params.boundary.se.elevation
		}
	);

	auto const ddy_0 = boundary_curve_interp(
		boundary_curve_interp_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.boundary.nw.ddy*ddy_scale,
			.y_m = params.boundary.n.ddy*ddy_scale,
			.y_1 = params.boundary.ne.ddy*ddy_scale
		}
	);

	auto const ddx_m = make_polynomial(
		cubic_spline_control_point{
			.y = params.boundary.w.ddy*ddy_scale,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.boundary.e.ddy*ddy_scale,
			.ddx = 0.0f
		}
	);

	auto const ddy_1 = boundary_curve_interp(
		boundary_curve_interp_descriptor{
			.x_m = params.edge_midpoints.n,
			.y_0 = params.boundary.sw.ddy*ddy_scale,
			.y_m = params.boundary.s.ddy*ddy_scale,
			.y_1 = params.boundary.se.ddy*ddy_scale
		}
	);

	auto const y_m_ns = make_polynomial(
		cubic_spline_control_point{
			.y = params.boundary.w.elevation,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.boundary.e.elevation,
			.ddx = 0.0f
		}
	);

	auto const y_m_we = make_polynomial(
		cubic_spline_control_point{
			.y = params.boundary.n.elevation,
			.ddx = 0.0f
		},
		cubic_spline_control_point{
			.y = params.boundary.s.elevation,
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

			auto const north_to_south = boundary_curve_interp(
				boundary_curve_interp_descriptor{
					.x_m = lerp(params.edge_midpoints.w, params.edge_midpoints.e, xi),
					.y_0 = west_to_east_north_interp(xi),
					.y_m = y_m_ns(xi),
					.y_1 = west_to_east_south_interp(xi)
				}
			);

			auto const north_to_south_grad = boundary_curve_grad(
				boundary_curve_grad_descriptor{
					.x_m = lerp(params.edge_midpoints.w, params.edge_midpoints.e, xi),
					.ddx_0 = ddy_0(xi),
					.ddx_m = ddx_m(xi),
					.ddx_1 = ddy_1(xi)
				}
			);

			auto const west_to_east = boundary_curve_interp(
				boundary_curve_interp_descriptor{
					.x_m = lerp(params.edge_midpoints.n, params.edge_midpoints.s, eta),
					.y_0 = north_to_south_west_interp(eta),
					.y_m = y_m_we(eta),
					.y_1 = north_to_south_east_interp(eta)
				}
			);
#if 0
			auto const west_to_east_grad = boundary_curve_grad(
				boundary_curve_grad_descriptor{
				.x_m = lerp(params.edge_midpoints.n, params.edge_midpoints.s, eta),
					.ddx_0 = std::lerp(params.boundary.nw.ddy, params.boundary.sw.ddy, eta)*ddy_scale,
					.ddx_m = std::lerp(params.boundary.n.ddy, params.boundary.s.ddy, eta)*ddy_scale,
					.ddx_1 = std::lerp(params.boundary.ne.ddy, params.boundary.se.ddy, eta)*ddy_scale
				}
			);
#endif

			ret.z_interp(x, y) = 0.5f*(north_to_south(eta) + west_to_east(xi));
			ret.z_grad(x, y) = north_to_south_grad(eta);
		}
	}

	return ret;
}

