//@	{"dependencies_extra":[{"ref":"./colorbar.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_COLORBAR_HPP
#define TERRAFORMER_UI_WIDGETS_COLORBAR_HPP

#include "./value_map.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

namespace terraformer::ui::widgets
{
	class colorbar:public main::widget_with_default_actions
	{
	public:
		colorbar()
		{ update_colorbar(); }

		using widget_with_default_actions::handle_event;

		template<class ValueMap, class ColorMap>
		explicit colorbar(ValueMap&& vm, ColorMap&& cm):
			m_value_map{std::forward<ValueMap>(vm)},
			m_color_map{std::forward<ColorMap>(cm)}
		{
			update_colorbar();
			update_frame();
		}

		void update_colorbar();

		void update_frame();

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		box_size compute_size(main::widget_width_request)
		{
			return box_size{
				static_cast<float>(m_image.frontend_resource().width()),
				static_cast<float>(m_image.frontend_resource().height()),
				0.0f
			};
		}

		box_size compute_size(main::widget_height_request)
		{
			return box_size{
				static_cast<float>(m_image.frontend_resource().width()),
				static_cast<float>(m_image.frontend_resource().height()),
				0.0f
			};
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info)
		{
			m_null_texture = cfg.misc_textures.null;
			m_fg_tint = cfg.output_area.colors.foreground;
			m_border_thickness = static_cast<uint32_t>(cfg.output_area.border_thickness);
			update_frame();
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

		main::unique_texture m_image;
		main::unique_texture m_frame;
		rgba_pixel m_fg_tint;
		unsigned int m_border_thickness = 0;
		main::immutable_shared_texture m_null_texture;
	};
}

#endif
