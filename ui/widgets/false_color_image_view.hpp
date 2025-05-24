//@	{"dependencies_extra":[{"ref":"./false_color_image_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_FALSE_COLOR_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_FALSE_COLOR_IMAGE_VIEW_HPP

#include "./value_map.hpp"
#include "./basic_image_view.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

namespace terraformer::ui::widgets
{
	class false_color_image_view:public basic_image_view<float, false_color_image_view>
	{
	public:
		false_color_image_view() = default;

		using basic_image_view<float, false_color_image_view>::handle_event;
		using basic_image_view<float, false_color_image_view>::compute_size;
		using basic_image_view<float, false_color_image_view>::confirm_size;
		using basic_image_view<float, false_color_image_view>::show_image;
		using basic_image_view<float, false_color_image_view>::prepare_for_presentation;

		template<class ValueMap, class ColorMap>
		explicit false_color_image_view(ValueMap&& vm, ColorMap&& cm):
			m_value_map{std::forward<ValueMap>(vm)},
			m_color_map{std::forward<ColorMap>(cm)}
		{ }

		image apply_filter(span_2d<float const> input) const;

		static rgba_pixel get_tint()
		{ return rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}; }

	private:
		type_erased_value_map m_value_map{
			std::in_place_type_t<value_maps::affine_value_map>{}, 0.0f, 1.0f
		};
		move_only_function<rgba_pixel(float)> m_color_map{
			[](float val){
				return rgba_pixel{val, val, val, 1.0f};
			}
		};
	};
}

#endif
