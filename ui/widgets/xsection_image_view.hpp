//@	{"dependencies_extra":[{"ref":"./xsection_image_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_XSECTION_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_XSECTION_IMAGE_VIEW_HPP

#include "./value_map.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"
#include "ui/main/texture_types.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

namespace terraformer::ui::widgets
{
	class xsection_image_view:public main::widget_with_default_actions
	{
	public:
		xsection_image_view() = default;

		template<class ValueMap, class ColorMap>
		explicit xsection_image_view(ValueMap&& vm, ColorMap&& cm):
			m_value_map{std::forward<ValueMap>(vm)},
			m_color_map{std::forward<ColorMap>(cm)}
		{ }

		void show_image(span_2d<float const> pixels);

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

	private:
		type_erased_value_map m_value_map{
			std::in_place_type_t<value_maps::affine_value_map>{}, 0.0f, 1.0f
		};
		move_only_function<rgba_pixel(float)> m_color_map{
			[](float val){
				return rgba_pixel{val, val, val, 1.0f};
			}
		};

		rgba_pixel m_bg_tint;
		main::immutable_shared_texture m_background;
		main::immutable_shared_texture m_null_texture;
	};
}

#endif
