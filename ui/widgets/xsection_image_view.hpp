//@	{"dependencies_extra":[{"ref":"./xsection_image_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_XSECTION_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_XSECTION_IMAGE_VIEW_HPP

#include "./value_map.hpp"
#include "lib/common/spaces.hpp"
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

		void show_image(span_2d<float const> image);

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		void set_physical_dimensions(float width, float height)
		{
			m_phys_width = width;
			m_phys_height = height;
			m_redraw_required = true;
		}

		void set_orientation(float new_val)
		{
			m_orientation = new_val;
			m_redraw_required = true;
		}

		box_size compute_size(main::widget_width_request wr)
		{
			auto const img_width = static_cast<float>(m_source_image.width());
			auto const img_height = static_cast<float>(m_source_image.width())*(m_max_val - m_min_val)/m_phys_width;
			auto const h = std::max(wr.height, 16.0f);
			auto const w = h*img_width/img_height;
			m_current_box = box_size{w, h, 0.0f};
			return box_size{w, h, 0.0f};
		}

		box_size compute_size(main::widget_height_request hr)
		{
			auto const img_width = static_cast<float>(m_source_image.width());
			auto const img_height = static_cast<float>(m_source_image.width())*(m_max_val - m_min_val)/m_phys_width;
			auto const w_temp = hr.width;
			auto const h = std::max(w_temp*img_height/img_width, 16.0f);
			auto const w = h*img_width/img_height;
			m_current_box = box_size{w, h, 0.0f};
			return box_size{w, h, 0.0f};
		}

		box_size confirm_size(box_size size)
		{
			auto const new_box = max(
				m_current_box.fit_xy_keep_z(
					size
				),
				box_size{1.0f, 1.0f, 0.0f}
			);
			return new_box;
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

		rgba_pixel m_bg_tint;
		main::immutable_shared_texture m_background;
		main::immutable_shared_texture m_null_texture;
		box_size m_current_box;
		grayscale_image m_source_image;
		float m_min_val;
		float m_max_val;
		float m_phys_width;
		float m_phys_height;
		float m_orientation;
		image m_diagram;
		bool m_redraw_required{false};
	};
}

#endif
