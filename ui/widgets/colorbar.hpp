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
		{ update_colorbar(); }

		void update_colorbar();

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		scaling compute_size(main::widget_width_request)
		{
			return scaling{
				static_cast<float>(m_image.frontend_resource().width()),
				static_cast<float>(m_image.frontend_resource().height()),
				1.0f
			};
		}

		scaling compute_size(main::widget_height_request)
		{
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
		main::immutable_shared_texture m_null_texture;
	};
}

#endif
