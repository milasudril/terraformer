//@	{"dependencies_extra":[{"ref":"./contour_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_CONTOUR_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_CONTOUR_VIEW_HPP

#include "./value_map.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

#include <mutex>

namespace terraformer::ui::widgets
{
	class contour_view:public main::widget_with_default_actions
	{
	public:
		contour_view() = default;

		using widget_with_default_actions::handle_event;

		explicit contour_view(float dz):
			m_dz{dz}
		{ }

		void show_image(span_2d<float const> image);

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		box_size compute_size(main::widget_width_request)
		{
			std::lock_guard lock{m_image_mutex};
			return box_size{
				static_cast<float>(m_image.frontend_resource().width()),
				static_cast<float>(m_image.frontend_resource().height()),
				0.0f
			};
		}

		box_size compute_size(main::widget_height_request)
		{
			std::lock_guard lock{m_image_mutex};
			return box_size{
				static_cast<float>(m_image.frontend_resource().width()),
				static_cast<float>(m_image.frontend_resource().height()),
				0.0f
			};
		}

		void handle_event(scaling)
		{}

		void theme_updated(main::config const& cfg, main::widget_instance_info)
		{
			m_null_texture = cfg.misc_textures.null;
			m_fg_tint = cfg.output_area.colors.foreground;
			m_border_thickness = static_cast<uint32_t>(cfg.output_area.border_thickness);
		}

	private:
		float m_dz;
		std::mutex m_image_mutex;
		main::unique_texture m_image{image{1, 1}};
		main::unique_texture m_frame{image{1 ,1}};
		rgba_pixel m_fg_tint;
		unsigned int m_border_thickness = 0;
		main::immutable_shared_texture m_null_texture;
	};
}

#endif
