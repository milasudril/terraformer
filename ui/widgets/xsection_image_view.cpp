//@	{"target":{"name":"xsection_image_view.o"}}

#include "./xsection_image_view.hpp"

#include "ui/drawing_api/image_generators.hpp"

#include "lib/common/function_ref.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/utils.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/value_maps/affine_value_map.hpp"

void terraformer::ui::widgets::xsection_image_view::show_image(span_2d<float const> image)
{
	m_source_image = grayscale_image{image};
	m_redraw_required = true;

	auto const size = static_cast<size_t>(image.width())*static_cast<size_t>(image.height());
	auto const minmax = std::ranges::minmax_element(image.data(), image.data() + size);
	auto const min = *minmax.min;
	auto const max = *minmax.max;

	m_min_val = min > 0.0f? 0.0f : -ceil_to_n_digits(-min, 2);
	m_max_val = max <= 0.0f? 0.0f : ceil_to_n_digits(max, 2);

	m_src_image_box_xy = box_size{
		static_cast<float>(m_source_image.width()),
		static_cast<float>(m_source_image.height()),
		0.0f
	};

	update_src_image_box_xz();
}

namespace
{
	struct draw_xsections_params
	{
		terraformer::box_size output_size;
		float xy_scale;
		float z_min;
		float z_max;
		float orientation;
		float rot_scale;
		terraformer::value_maps::affine_value_map depth_value_map;
		terraformer::function_ref<terraformer::rgba_pixel(float)> color_map;
	};

	struct xsection_point
	{
		float z;
	};

	struct xsection_point_output_params
	{
		float z_min;
		float z_max;
		float image_height;
		float cos_theta;
		float sin_theta;
		float rot_scale;
	};

	xsection_point get_xsection_point(
		terraformer::span_2d<float const> input,
		float x_in,
		float y_in,
		xsection_point_output_params const& params_out
	)
	{
		auto const w = static_cast<float>(input.width());
		auto const h = static_cast<float>(input.height());

		auto const x_in_centered = x_in - 0.5f*w;
		auto const y_in_centered = y_in - 0.5f*h;
		auto const x_rot = params_out.rot_scale*(
			 params_out.cos_theta*x_in_centered + params_out.sin_theta*y_in_centered
		) + 0.5f*w;
		auto const y_rot = params_out.rot_scale*(
			- params_out.sin_theta*x_in_centered + params_out.cos_theta*y_in_centered
		) + 0.5f*h;

		auto const z_in = interp(
			input,
			x_rot - 0.5f,
			y_rot - 0.5f,
			terraformer::clamp_at_boundary{}
		);
		return xsection_point{
			.z = params_out.image_height*(params_out.z_max - z_in)/(params_out.z_max - params_out.z_min),
		};
	}

	void draw_blob(
		terraformer::span_2d<terraformer::rgba_pixel> output,
		int32_t x_0,
		int32_t y_0,
		terraformer::rgba_pixel color
	)
	{
		output(x_0, y_0) = color;
		output(std::min(x_0 + 1, static_cast<int32_t>(output.width()) - 1), y_0) = color;
		output(x_0, std::min(y_0 + 1, static_cast<int32_t>(output.height()) - 1)) = color;
		output(
			std::min(x_0 + 1, static_cast<int32_t>(output.width()) - 1),
			std::min(y_0 + 1, static_cast<int32_t>(output.height()) - 1)
		) = color;
	}

	void draw_line(
		terraformer::span_2d<terraformer::rgba_pixel> output,
		int32_t x_0,
		xsection_point p_0,
		xsection_point p_1,
		terraformer::rgba_pixel color
	)
	{
		auto const dz = p_1.z - p_0.z;
		constexpr auto dx = 1.0f;

		if(dx > std::abs(dz))
		{
			auto const z_out = p_0.z;
			draw_blob(output, x_0, static_cast<int32_t>(z_out), color);
		}
		else
		{
			auto step = p_0.z >= p_1.z? -1 : 1;
			auto const z_0 = static_cast<int32_t>(p_0.z);
			for(auto z = z_0; z != static_cast<int32_t>(p_1.z); z += step)
			{
				auto const x_out = static_cast<float>(x_0) + dx*static_cast<float>(z - z_0)/dz;
				draw_blob(output, static_cast<int32_t>(x_out), z, color);
			}
		}
	}

	terraformer::image draw_cross_sections(
		terraformer::span_2d<float const> input,
		draw_xsections_params const& params
	)
	{
		auto const w = static_cast<uint32_t>(params.output_size[0] + 0.5f);
		auto const h = static_cast<uint32_t>(params.output_size[1] + 0.5f);
		assert(h >= 1);
		assert(w >= 1);

		terraformer::image ret{w, h};
		auto const slice_count = 8.0f*params.rot_scale;
		auto const dy = static_cast<float>(input.height())/slice_count;

		xsection_point_output_params const output_params{
			.z_min = params.z_min,
			.z_max = params.z_max,
			.image_height = static_cast<float>(h),
			.cos_theta = std::cos(params.orientation),
			.sin_theta = std::sin(params.orientation),
			.rot_scale = params.rot_scale
		};

		for(size_t k = 0; k != static_cast<size_t>(slice_count + 0.5f); ++k)
		{
			auto const x_in = 0.5f*params.xy_scale;
			auto const y_in = (static_cast<float>(k) + 0.5f)*dy;
			auto p_0 = get_xsection_point(
				input,
				x_in,
				y_in,
				output_params
			);

			auto const color = params.color_map(
				std::clamp(1.0f - params.depth_value_map.from_value(y_in), 0.0f, 1.0f)
			);

			for(uint32_t x_out = 1; x_out != w; ++x_out)
			{
				auto const x_in = (static_cast<float>(x_out) + 0.5f)*params.xy_scale;
				auto const p_1 = get_xsection_point(
					input,
					x_in,
					y_in,
					output_params
				);

				draw_line(ret, x_out - 1, p_0, p_1, color);
				p_0 = p_1;
			}
		}

		return ret;
	}
}

terraformer::ui::main::widget_layer_stack terraformer::ui::widgets::xsection_image_view::prepare_for_presentation(main::graphics_backend_ref backend)
{
	std::array const bg_tints{
		m_cfg.bg_tint,
		m_cfg.bg_tint,
		m_cfg.bg_tint,
		m_cfg.bg_tint
	};
	auto const null_texture = m_cfg.null_texture->get_backend_resource(backend).get();
	auto const background = m_cfg.background->get_backend_resource(backend).get();
	auto const border_displacement = m_cfg.border_thickness*displacement{1.0f, 1.0f, 0.0f};

	if(m_redraw_required)
	{
		m_diagram = draw_cross_sections(
			m_source_image.pixels(),
			draw_xsections_params{
				.output_size = m_adjusted_box,
				.xy_scale = m_src_image_box_xy[0]/m_adjusted_box[0],
				.z_min = m_min_val,
				.z_max = m_max_val,
				.orientation = m_orientation,
				.rot_scale = m_rot_scale,
				.depth_value_map = m_value_map,
				.color_map = m_color_map.ref()
			}
		);

		auto const full_box = m_adjusted_box + 2.0f*border_displacement;
		auto const w = static_cast<uint32_t>(full_box[0]);
		auto const h = static_cast<uint32_t>(full_box[1]);

		m_frame = generate(
			drawing_api::flat_rectangle{
				.domain_size = span_2d_extents {
					.width = w,
					.height = h
				},
				.origin_x = 0u,
				.origin_y = 0u,
				.width = w,
				.height = h,
				.border_thickness = static_cast<uint32_t>(m_cfg.border_thickness),
				.border_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				.fill_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
			}
		);

		m_redraw_required = false;
	}

	std::array const fg_tints{
		rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
		rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
		rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
		rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
	};

	std::array const frame_tints{
		m_cfg.frame_tint,
		m_cfg.frame_tint,
		m_cfg.frame_tint,
		m_cfg.frame_tint
	};

	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = background,
			.tints = bg_tints
		},
		.sel_bg_mask = main::widget_layer_mask{
			.offset = displacement{},
			.texture = null_texture,
		},
		.selection_background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.foreground = main::widget_layer{
			.offset = border_displacement,
			.rotation = geosimd::turn_angle{},
			.texture = m_diagram.get_backend_resource(backend).get(),
			.tints = fg_tints
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_frame.get_backend_resource(backend).get(),
			.tints = frame_tints
		},
		.input_marker = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		}
	};
}
