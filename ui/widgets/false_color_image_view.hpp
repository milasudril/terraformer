//@	{"dependencies_extra":[{"ref":"./false_color_image_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_FALSE_COLOR_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_FALSE_COLOR_IMAGE_VIEW_HPP

#include "./value_map.hpp"
#include "./image_view.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

#include <mutex>

namespace terraformer::ui::widgets
{
	class false_color_image_view:
		public main::widget_with_default_actions,
		public image_view<grayscale_image>
	{
	public:
		false_color_image_view() = default;

		using widget_with_default_actions::handle_event;

		template<class ValueMap, class ColorMap>
		explicit false_color_image_view(ValueMap&& vm, ColorMap&& cm):
			m_value_map{std::forward<ValueMap>(vm)},
			m_color_map{std::forward<ColorMap>(cm)}
		{ }

		void show_image(span_2d<float const> image);

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		box_size compute_size(main::widget_width_request wr)
		{
			std::lock_guard lock{m_image_mutex};
			auto const img_width = static_cast<float>(m_image.frontend_resource().width());
			auto const img_height = static_cast<float>(m_image.frontend_resource().height());
			auto const h = std::max(wr.height, m_min_img_height);
			auto const w = h*img_width/img_height;
			return box_size{w, h, 0.0f};
		}

		box_size compute_size(main::widget_height_request hr)
		{
			std::lock_guard lock{m_image_mutex};
			auto const img_width = static_cast<float>(m_image.frontend_resource().width());
			auto const img_height = static_cast<float>(m_image.frontend_resource().height());;
			auto const w_temp = hr.width;
			auto const h = std::max(w_temp*img_height/img_width, m_min_img_height);
			auto const w = h*img_width/img_height;
			return box_size{w, h, 0.0f};
		}

		box_size confirm_size(box_size size)
		{
			return size;
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info)
		{
			m_null_texture = cfg.misc_textures.null;
			m_fg_tint = cfg.output_area.colors.foreground;
			m_border_thickness = static_cast<uint32_t>(cfg.output_area.border_thickness);
			m_min_img_height = cfg.output_area.min_img_height;
		}

	private:
		type_erased_value_map m_value_map{
			std::in_place_type_t<value_maps::affine_value_map>{}, 0.0f, 1.0f
		};
		move_only_function<rgba_pixel(float)> m_color_map{
			[](float val){
				return rgba_pixel{val, val, val, 1.0f};
			}
		};
		std::mutex m_image_mutex;
		main::unique_texture m_image{image{1, 1}};
		main::unique_texture m_frame{image{1 ,1}};
		rgba_pixel m_fg_tint;
		unsigned int m_border_thickness = 0;
		float m_min_img_height;
		main::immutable_shared_texture m_null_texture;
	};
}

#endif
