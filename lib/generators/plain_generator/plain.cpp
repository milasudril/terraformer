//@	{"target": {"name": "./plain.o"}}

#include "./plain.hpp"

#include "lib/common/interval.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/value_map.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/value_maps/atan_value_map.hpp"
#include "lib/value_maps/sqrt_value_map.hpp"

#include <stdexcept>

namespace
{
	struct control_curve_descriptor
	{
		terraformer::bounded_value<terraformer::open_open_interval{0.0f, 1.0f}, 0.5f> x_m{0.5f};

		float y_0 = 0.0f;
		float y_m = 0.0f;
		float y_1 = 0.0f;
		float ddx_0 = 0.0f;
		float ddx_m = 0.0f;
		float ddx_1 = 0.0f;
	};

	class control_curve
	{
	public:
		constexpr explicit control_curve(control_curve_descriptor const& bcd):
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
	domain_size_descriptor dom_size,
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

	auto const west_to_east_north = control_curve(
		control_curve_descriptor{
			.x_m = params.midpoints.n,
			.y_0 = params.control_points.nw.elevation,
			.y_m = params.control_points.n.elevation,
			.y_1 = params.control_points.ne.elevation,
			.ddx_0 = params.control_points.nw.ddx*dom_size.width,
			.ddx_m = params.control_points.n.ddx*dom_size.width,
			.ddx_1 = params.control_points.ne.ddx*dom_size.width
		}
	);

	auto const west_to_east_south = control_curve(
		control_curve_descriptor{
			.x_m = params.midpoints.s,
			.y_0 = params.control_points.sw.elevation,
			.y_m = params.control_points.s.elevation,
			.y_1 = params.control_points.se.elevation,
			.ddx_0 = params.control_points.sw.ddx*dom_size.width,
			.ddx_m = params.control_points.s.ddx*dom_size.width,
			.ddx_1 = params.control_points.se.ddx*dom_size.width
		}
	);

	auto const north_to_south_west = control_curve(
		control_curve_descriptor{
			.x_m = params.midpoints.w,
			.y_0 = params.control_points.nw.elevation,
			.y_m = params.control_points.w.elevation,
			.y_1 = params.control_points.sw.elevation,
			.ddx_0 = params.control_points.nw.ddy*dom_size.height,
			.ddx_m = params.control_points.w.ddy*dom_size.height,
			.ddx_1 = params.control_points.sw.ddy*dom_size.height
		}
	);

	auto const north_to_south_east = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.e,
			.y_0 = params.control_points.ne.elevation,
			.y_m = params.control_points.e.elevation,
			.y_1 = params.control_points.se.elevation,
			.ddx_0 = params.control_points.ne.ddy*dom_size.height,
			.ddx_m = params.control_points.e.ddy*dom_size.height,
			.ddx_1 = params.control_points.se.ddy*dom_size.height
		}
	};

	auto const z_m_interp_ns = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.c_x,
			.y_0 = params.control_points.w.elevation,
			.y_m = params.control_points.c.elevation,
			.y_1 = params.control_points.e.elevation,
			.ddx_0 = params.control_points.w.ddx*dom_size.width,
			.ddx_m = params.control_points.c.ddx*dom_size.width,
			.ddx_1 = params.control_points.e.ddx*dom_size.width
		}
	};

	auto const z_m_interp_we = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.c_y,
			.y_0 = params.control_points.n.elevation,
			.y_m = params.control_points.c.elevation,
			.y_1 = params.control_points.s.elevation,
			.ddx_0 = params.control_points.n.ddy*dom_size.height,
			.ddx_m = params.control_points.c.ddy*dom_size.height,
			.ddx_1 = params.control_points.s.ddy*dom_size.height
		}
	};

	auto const y_m = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.c_x,
			.y_0 = params.midpoints.w,
			.y_m = params.midpoints.c_y,
			.y_1 = params.midpoints.e,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const x_m = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.c_y,
			.y_0 = params.midpoints.n,
			.y_m = params.midpoints.c_x,
			.y_1 = params.midpoints.s,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const ddy_0 = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.n,
			.y_0 = params.control_points.nw.ddy*dom_size.height,
			.y_m = params.control_points.n.ddy*dom_size.height,
			.y_1 = params.control_points.ne.ddy*dom_size.height,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const ddy_m = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.c_x,
			.y_0 = params.control_points.w.ddy*dom_size.height,
			.y_m = params.control_points.c.ddy*dom_size.height,
			.y_1 = params.control_points.e.ddy*dom_size.height,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f,
		}
	};

	auto const ddy_1 = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.n,
			.y_0 = params.control_points.sw.ddy*dom_size.height,
			.y_m = params.control_points.s.ddy*dom_size.height,
			.y_1 = params.control_points.se.ddy*dom_size.height,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const ddx_0 = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.n,
			.y_0 = params.control_points.nw.ddx*dom_size.width,
			.y_m = params.control_points.w.ddx*dom_size.width,
			.y_1 = params.control_points.sw.ddx*dom_size.width,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const ddx_m = control_curve{
		control_curve_descriptor{
			.x_m = params.midpoints.c_y,
			.y_0 = params.control_points.n.ddx*dom_size.width,
			.y_m = params.control_points.c.ddx*dom_size.width,
			.y_1 = params.control_points.s.ddx*dom_size.width,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	};

	auto const ddx_1 = control_curve{		control_curve_descriptor{
			.x_m = params.midpoints.n,
			.y_0 = params.control_points.ne.ddx*dom_size.width,
			.y_m = params.control_points.e.ddx*dom_size.width,
			.y_1 = params.control_points.se.ddx*dom_size.width,
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

			auto const north_to_south = control_curve(
				control_curve_descriptor{
				.x_m = bounded_value<open_open_interval{0.0f, 1.0f}, 0.5f>{y_m(xi)},
				.y_0 = west_to_east_north(xi),
				.y_m = z_m_interp_ns(xi),
				.y_1 = west_to_east_south(xi),
				.ddx_0 = ddy_0(xi),
				.ddx_m = ddy_m(xi),
				.ddx_1 = ddy_1(xi)
				}
			);

			auto const west_to_east = control_curve(
				control_curve_descriptor{
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

void terraformer::plain_control_point_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"Elevation/m",
		elevation,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::sqrt_value_map{6400.0f}},
			.textbox_placeholder_string = u8"-9999.9999",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"∂/∂x",
		std::ref(ddx),
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::atan_value_map{1.0f, true}},
			.textbox_placeholder_string = u8"-0.073242545",
			.visual_angle_range = closed_closed_interval<geosimd::turn_angle>{
					geosimd::turns{0.5 - 0.125},
					geosimd::turns{0.5 + 0.125}
			}
		}
	);

	editor.create_float_input(
		u8"∂/∂y",
		std::ref(ddy),
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::atan_value_map{1.0f, true}},
			.textbox_placeholder_string = u8"-0.073242545",
			.visual_angle_range = closed_closed_interval<geosimd::turn_angle>{
					geosimd::turns{0.5 - 0.125},
					geosimd::turns{0.5 + 0.125}
			}
		});
}

void terraformer::plain_control_points_info::bind(descriptor_table_editor_ref editor)
{
	{
		auto n_edit = editor.add_record(u8"N");
		n.bind(n_edit);
		n_edit.append_pending_widgets();
	}

	{
		auto ne_edit = editor.add_record(u8"NE");
		ne.bind(ne_edit);
		ne_edit.append_pending_widgets();
	}

	{
		auto e_edit =editor.add_record(u8"E");
		ne.bind(e_edit);
		e_edit.append_pending_widgets();
	}

	{
		auto se_edit = editor.add_record(u8"SE");
		se.bind(se_edit);
		se_edit.append_pending_widgets();
	}

	{
		auto s_edit = editor.add_record(u8"S");
		s.bind(s_edit);
		s_edit.append_pending_widgets();
	}

	{
		auto sw_edit = editor.add_record(u8"SW");
		sw.bind(sw_edit);
		sw_edit.append_pending_widgets();
	}

	{
		auto w_edit = editor.add_record(u8"W");
		w.bind(w_edit);
		w_edit.append_pending_widgets();
	}

	{
		auto nw_edit = editor.add_record(u8"NW");
		nw.bind(nw_edit);
		nw_edit.append_pending_widgets();
	}

	{
		auto c_edit = editor.add_record(u8"C");
		c.bind(c_edit);
		c_edit.append_pending_widgets();
	}
}

void terraformer::plain_midpoints_info::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"N",
		descriptor_editor_ref::assigner<float>{n},
		descriptor_editor_ref::knob_descriptor{
			.textbox_placeholder_string = u8"0.00019329926",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"E",
		descriptor_editor_ref::assigner<float>{e},
		descriptor_editor_ref::knob_descriptor{
			.textbox_placeholder_string = u8"0.00019329926",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"S",
		descriptor_editor_ref::assigner<float>{s},
		descriptor_editor_ref::knob_descriptor{
			.textbox_placeholder_string = u8"0.00019329926",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"W",
		descriptor_editor_ref::assigner<float>{w},
		descriptor_editor_ref::knob_descriptor{
			.textbox_placeholder_string = u8"0.00019329926",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"C x",
		descriptor_editor_ref::assigner<float>{c_x},
		descriptor_editor_ref::knob_descriptor{
			.textbox_placeholder_string = u8"0.00019329926",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"C y",
		descriptor_editor_ref::assigner<float>{c_y},
		descriptor_editor_ref::knob_descriptor{
			.textbox_placeholder_string = u8"0.00019329926",
			.visual_angle_range = std::nullopt
		}
	);
}

terraformer::grayscale_image terraformer::plain_descriptor::generate_heightmap(domain_size_descriptor size) const
{ return generate(size, *this); }

void terraformer::plain_descriptor::bind(descriptor_editor_ref editor)
{
	auto control_points_editor = editor.create_table(
		u8"Control points",
		descriptor_editor_ref::table_descriptor{
			.orientation = descriptor_editor_ref::widget_orientation::vertical,
			.field_names = {
				u8"Elevation/m",
				u8"∂/∂x",
				u8"∂/∂y"
			}
		}
	);
	control_points.bind(control_points_editor);

	auto midpoints_editor = editor.create_form(
		u8"Midpoints",
		descriptor_editor_ref::form_descriptor{
			.orientation = descriptor_editor_ref::widget_orientation::vertical,
			.extra_fields_per_row = 1
		}
	);
	midpoints.bind(midpoints_editor);

	editor.create_float_input(
		u8"Orientation",
		std::ref(orientation),
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::affine_value_map{-0.5f, 0.5f}},
			.textbox_placeholder_string = u8"-0.123456789",
			.visual_angle_range = closed_closed_interval<geosimd::turn_angle>{
				geosimd::turns{0.0},
				geosimd::turns{1.0}
			}
		}
	);
}


