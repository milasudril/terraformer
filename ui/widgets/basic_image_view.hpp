#ifndef TERRAFORMER_UI_WIDGETS_BASIC_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_BASIC_IMAGE_VIEW_HPP

#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"
#include "ui/drawing_api/image_generators.hpp"

#include "lib/pixel_store/image.hpp"

namespace terraformer::ui::widgets
{
	struct basic_image_view_config
	{
		main::immutable_shared_texture null_texture;
		rgba_pixel fg_tint;
		float border_thickness;
		float min_img_height;
	};

	template<class PixelType, class PresentationFilter>
	class basic_image_view:public main::widget_with_default_actions
	{
	public:
		void show_image(span_2d<PixelType const> image)
		{
			m_current_box = box_size{
				static_cast<float>(image.width()),
				static_cast<float>(image.height()),
				0.0f
			};
			m_current_image = basic_image{image};
			m_source_image_dirty = true;
		}

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend)
		{
			if(m_source_image_dirty)
			{
			//	auto const resized_image = resize(m_current_image, m_adjusted_box);
				m_image = static_cast<PresentationFilter const&>(*this).apply_filter(m_current_image.pixels());
				auto const full_box = m_adjusted_box + 2.0f*m_cfg.border_thickness*displacement{1.0f, 1.0f, 0.0f};
				auto const w = static_cast<uint32_t>(full_box[0]);
				auto const h = static_cast<uint32_t>(full_box[1]);
				m_frame =  generate(
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
				m_source_image_dirty = false;
			}

			auto const null_texture = m_cfg.null_texture->get_backend_resource(backend).get();
			auto const fg_tint = m_cfg.fg_tint;

			return main::widget_layer_stack{
				.background = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				},
				.sel_bg_mask = main::widget_layer_mask{
					.offset = displacement{},
					.texture = null_texture
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
					.texture = m_image.get_backend_resource(backend).get(),
					.tints = std::array<rgba_pixel, 4>{
						rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
						rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
						rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
						rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
					}
				},
				.frame = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = m_frame.get_backend_resource(backend).get(),
					.tints = std::array{fg_tint, fg_tint, fg_tint, fg_tint}
				},
				.input_marker{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				}
			};
		}

		bool is_source_image_dirty() const
		{ return m_source_image_dirty; }

		span_2d<PixelType const> current_image() const
		{ return m_current_image.pixels(); }

		auto const& config() const
		{ return m_cfg; }

		box_size compute_size(main::widget_width_request wr)
		{
			auto const img_width = m_current_box[0];
			auto const img_height = m_current_box[1];
			auto const h = std::max(wr.height, m_cfg.min_img_height);
			auto const w = h*img_width/img_height;
			return box_size{w, h, 0.0f} + 2.0f*m_cfg.border_thickness*displacement{1.0f, 1.0f, 0.0f};
		}

		box_size compute_size(main::widget_height_request hr)
		{
			auto const img_width = m_current_box[0];
			auto const img_height = m_current_box[1];
			auto const w_temp = hr.width;
			auto const h = std::max(w_temp*img_height/img_width, m_cfg.min_img_height);
			auto const w = h*img_width/img_height;
			return box_size{w, h, 0.0f} + 2.0f*m_cfg.border_thickness*displacement{1.0f, 1.0f, 0.0f};
		}

		box_size confirm_size(box_size size)
		{
			auto const new_box = m_current_box.fit_xy_keep_z(
				size + 2.0f*m_cfg.border_thickness*displacement{-1.0f, -1.0f, 0.0f}
			);
			m_source_image_dirty = (new_box != m_adjusted_box);
			m_adjusted_box = new_box;
			return new_box + 2.0f*m_cfg.border_thickness*displacement{1.0f, 1.0f, 0.0f};
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info)
		{
			m_cfg.null_texture = cfg.misc_textures.null;
			m_cfg.fg_tint = cfg.output_area.colors.foreground;
			m_cfg.border_thickness = cfg.output_area.border_thickness;
			m_cfg.min_img_height = cfg.output_area.min_img_height;

			m_source_image_dirty = true;
		}

	private:
		basic_image<PixelType> m_current_image;
		image m_filtered_image;
		box_size m_current_box;
		box_size m_adjusted_box;
		bool m_source_image_dirty = false;
		main::unique_texture m_image{image{1, 1}};
		main::unique_texture m_frame{image{1 ,1}};

		basic_image_view_config m_cfg;

	};
}

#endif