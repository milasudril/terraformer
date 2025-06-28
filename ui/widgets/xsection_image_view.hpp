//@	{"dependencies_extra":[{"ref":"./xsection_image_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_XSECTION_IMAGE_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_XSECTION_IMAGE_VIEW_HPP

#include "./value_map.hpp"
#include "lib/common/spaces.hpp"
#include "lib/math_utils/trigfunc.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"
#include "ui/main/texture_types.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

namespace terraformer::ui::widgets
{
	struct xsection_image_view_config
	{
		main::immutable_shared_texture null_texture;
		main::immutable_shared_texture background;
		rgba_pixel bg_tint;
		float border_thickness;
		float min_img_height;
	};

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

		void theme_updated(main::config const& cfg, main::widget_instance_info)
		{
			m_cfg.null_texture = cfg.misc_textures.null;
			m_cfg.background = cfg.misc_textures.white;
			m_cfg.bg_tint = cfg.output_area.colors.background;
			m_cfg.border_thickness = cfg.output_area.border_thickness;
			m_cfg.min_img_height = cfg.output_area.min_img_height;

			m_redraw_required = true;
		}

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		void set_physical_dimensions(float width, float height)
		{
			m_phys_width = width;
			m_phys_height = height;
			update_current_box();
		}

		void set_orientation(float new_val)
		{
			m_orientation = new_val;
			update_current_box();
		}

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
			auto const new_box = max(
				m_current_box.fit_xy_keep_z(
					size + 2.0f*m_cfg.border_thickness*displacement{-1.0f, -1.0f, 0.0f}
				),
				box_size{1.0f, 1.0f, 0.0f}
			);
			m_redraw_required = (new_box != m_adjusted_box || m_redraw_required);
			m_adjusted_box = new_box;
			return new_box + 2.0f*m_cfg.border_thickness*displacement{1.0f, 1.0f, 0.0f};
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

		void update_current_box()
		{
			auto const r = distance_from_origin_to_edge_xy(
				m_src_image_box, 2.0f*std::numbers::pi_v<float>*m_orientation
			);
			auto const width = r;
			auto const height = m_src_image_box[0]*(m_max_val - m_min_val)/m_phys_width;
			m_current_box = box_size{width, height, 0.0f};
			m_redraw_required = true;
		}

		xsection_image_view_config m_cfg;
		box_size m_current_box;
		box_size m_adjusted_box;
		main::immutable_shared_texture m_background;

		grayscale_image m_source_image;
		float m_min_val;
		float m_max_val;
		box_size m_src_image_box;

		float m_phys_width;
		float m_phys_height;
		float m_orientation;
		image m_diagram;
		bool m_redraw_required{false};
	};
}

#endif
