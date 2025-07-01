//@	{"target":{"name":"xsection_image_view.o"}}

#include "./xsection_image_view.hpp"

#include "lib/common/function_ref.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/utils.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"
#include "ui/value_maps/affine_value_map.hpp"

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
		terraformer::ui::value_maps::affine_value_map depth_value_map;
		terraformer::function_ref<terraformer::rgba_pixel(float)> color_map;
	};

	struct xsection_point
	{
		uint32_t x;
		float z;
	};

	struct xsection_point_output_params
	{
		uint32_t x;
		float z_min;
		float z_max;
		float image_height;
	};

	xsection_point get_xsection_point(
		xsection_point_output_params const& params_out,
		terraformer::span_2d<float const> input,
		float x_in,
		float y_in
	)
	{
		auto const z_in = interp(input, x_in - 0.5f, y_in - 0.5f, terraformer::clamp_at_boundary{});
		return xsection_point{
			.x = params_out.x,
			.z = params_out.image_height*(params_out.z_max - z_in)/(params_out.z_max - params_out.z_min),
		};
	}

	void draw_line(
		terraformer::span_2d<terraformer::rgba_pixel> output,
		xsection_point p_0,
		xsection_point p_1,
		terraformer::rgba_pixel color
	)
	{
		assert(p_0.x < p_1.x);
		auto const dz = p_1.z - p_0.z;
		auto const dx = static_cast<float>(p_1.x - p_0.x);

		if(dx > dz)
		{
			for(auto x = p_0.x; x < p_1.x; ++x)
			{
				auto const z_out = p_0.z + dz*static_cast<float>(x - p_0.x)/dx;
				output(x, std::min(static_cast<uint32_t>(z_out), output.height() - 1)) = color;
			}
		}
		else
		{
			auto step = p_0.z >= p_1.z? -1 : 1;
			auto const z_0 = static_cast<int32_t>(p_0.z);
			for(auto z = z_0; z != static_cast<int32_t>(p_1.z); z += step)
			{
				auto const x_out = static_cast<float>(p_0.x) + dx*static_cast<float>(z - z_0)/dz;
				output(static_cast<uint32_t>(x_out), z) = color;
			}
		}
	}

	terraformer::image draw_cross_sections(
		terraformer::span_2d<float const> input,
		draw_xsections_params const& params
	)
	{
		auto const w_float = params.output_size[0];
		auto const w = static_cast<uint32_t>(w_float + 0.5f);
		auto const h = static_cast<uint32_t>(params.output_size[1] + 0.5f);
		assert(h >= 1);
		assert(w >= 1);

		terraformer::image ret{w, h};
		size_t const slice_count = 8;
		auto const dy = static_cast<float>(input.height())/static_cast<float>(slice_count);

		for(size_t k = 0; k != slice_count; ++k)
		{
			auto const x_in = 0.5f*params.xy_scale;
			auto const y_in = (static_cast<float>(k) + 0.5f)*dy;
			auto p_0 = get_xsection_point(
				xsection_point_output_params{
					.x = 0,
					.z_min = params.z_min,
					.z_max = params.z_max,
					.image_height = static_cast<float>(h)
				},
				input,
				x_in,
				y_in
			);

			auto const color = params.color_map(
				std::clamp(1.0f - params.depth_value_map.from_value(y_in), 0.0f, 1.0f)
			);

			for(uint32_t x_out = 1; x_out != w; ++x_out)
			{
				auto const x_in = (static_cast<float>(x_out) + 0.5f)*params.xy_scale;
				auto const p_1 = get_xsection_point(
					xsection_point_output_params{
						.x = x_out,
						.z_min = params.z_min,
						.z_max = params.z_max,
						.image_height = static_cast<float>(h)
					},
					input,
					x_in,
					y_in
				);

				draw_line(ret, p_0, p_1, color);
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
				.depth_value_map = m_value_map,
				.color_map = m_color_map.ref()
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
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_diagram.get_backend_resource(backend).get(),
			.tints = fg_tints
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			// TODO: Will be used to add a frame around the image
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.input_marker = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		}
	};
}
