#ifndef TERRAFORMER_UI_WIDGETS_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_IMAGE_VIEW_HPP

#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

#include "lib/pixel_store/image.hpp"

namespace terraformer::ui::widgets
{
	struct image_view_config
	{
		main::immutable_shared_texture null_texture;
		rgba_pixel fg_tint;
		float border_thickness;
		float min_img_height;
	};

	template<class PixelType>
	class image_view
	{
	public:
		void show_image(basic_image<PixelType>&& image)
		{
			m_current_box = box_size{
				static_cast<float>(image.width()),
				static_cast<float>(image.height()),
				0.0f
			};
			m_current_image = std::move(image);
			m_source_image_dirty = true;
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
			m_adjusted_box = m_current_box.fit_xy_keep_z(size);
			return m_adjusted_box + 2.0f*m_cfg.border_thickness*displacement{1.0f, 1.0f, 0.0f};
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info)
		{
			m_cfg.null_texture = cfg.misc_textures.null;
			m_cfg.fg_tint = cfg.output_area.colors.foreground;
			m_cfg.border_thickness = cfg.output_area.border_thickness;
			m_cfg.min_img_height = cfg.output_area.min_img_height;
		}

	private:
		basic_image<PixelType> m_current_image;
		box_size m_current_box;
		box_size m_adjusted_box;
		bool m_source_image_dirty = false;

		image_view_config m_cfg;

	};
}

#endif