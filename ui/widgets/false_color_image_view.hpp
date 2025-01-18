//@	{"dependencies_extra":[{"ref":"./false_color_image_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_FALSE_COLOR_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_FALSE_COLOR_IMAGE_VIEW_HPP

#include "./value_map.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"

namespace terraformer::ui::widgets
{
	class false_color_image_view:public main::widget_with_default_actions
	{
	public:
		false_color_image_view() = default;

		template<class ValueMap, class ColorMap>
		explicit false_color_image_view(ValueMap&& vm, ColorMap&& cm):
			m_value_map{std::forward<ValueMap>(vm)},
			m_color_map{std::forward<ColorMap>(vm)}
		{ }

		void regenerate_textures();

		void show_image(span_2d<float const> image);

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size)
		{}

		void theme_updated(main::config const& cfg, main::widget_instance_info);

	private:
		type_erased_value_map m_value_map;
		move_only_function<rgba_pixel(float)> m_color_map;
		main::unique_texture m_image;
		main::immutable_shared_texture m_null_texture;
	};
}

#endif
