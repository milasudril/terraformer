//@	{"dependencies_extra":[{"ref":"./false_color_image_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_FALSE_COLOR_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_FALSE_COLOR_IMAGE_VIEW_HPP

#include "./value_map.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

#include <mutex>

namespace terraformer::ui::widgets
{
	class false_color_image_view:public main::widget_with_default_actions
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

		scaling compute_size(main::widget_width_request)
		{
			std::lock_guard lock{m_image_mutex};
			return scaling{
				static_cast<float>(m_image.frontend_resource().width()),
				static_cast<float>(m_image.frontend_resource().height()),
				1.0f
			};
		}

		scaling compute_size(main::widget_height_request)
		{
			std::lock_guard lock{m_image_mutex};
			return scaling{
				static_cast<float>(m_image.frontend_resource().width()),
				static_cast<float>(m_image.frontend_resource().height()),
				1.0f
			};
		}

		void handle_event(main::fb_size)
		{}

		void theme_updated(main::config const& cfg, main::widget_instance_info)
		{
			m_null_texture = cfg.misc_textures.null;
			m_fg_tint = cfg.output_area.colors.foreground;
			m_border_thickness = static_cast<uint32_t>(cfg.input_area.border_thickness);
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
		main::immutable_shared_texture m_null_texture;
	};
}

#endif
