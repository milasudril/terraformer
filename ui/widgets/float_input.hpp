#ifndef TERRAFORMER_UI_WIDGETS_FLOAT_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_FLOAT_INPUT_HPP

#include "./text_to_float_input.hpp"
#include "ui/layouts/rowmajor_table.hpp"
#include "ui/main/widget_collection.hpp"

namespace terraformer::ui::widgets
{
	template<class ControlWidget>
	class float_input:public main::widget_with_default_actions
	{
	public:
		template<class Function>
		float_input& on_value_changed(Function&& f)
		{
			m_on_value_changed = std::forward<Function>(f);
			return *this;
		}

		float value() const
		{ return m_input_widget.value(); }

		float_input& value(float new_val)
		{
			m_input_widget.value(new_val);
			m_textbox.value(new_val);
			return *this;
		}

		// TODO: It could be a good idea to only show ControlWidget when we have keyboard/mouse focus
		// //       This will save precious space

		float_input()
		{
			m_widgets.append(std::ref(m_input_widget), terraformer::ui::main::widget_geometry{});
			m_widgets.append(std::ref(m_textbox), terraformer::ui::main::widget_geometry{});
			m_input_widget.on_value_changed([this]<class ... Args>(auto const& input, Args&&... args){
				m_textbox.value(input.value());
				m_on_value_changed(*this, std::forward<Args>(args)...);
			});
			m_textbox
				.on_value_changed([this]<class ... Args>(auto& input, Args&&... args){
					m_input_widget.value(input.value());
					input.value(m_input_widget.value());
					m_on_value_changed(*this, std::forward<Args>(args)...);
				})
				.value(m_input_widget.value());
		}

		main::layout_policy_ref get_layout()
		{ return main::layout_policy_ref{std::ref(m_layout)}; }

		main::widget_collection_ref get_children()
		{ return m_widgets.get_attributes(); }

		main::widget_collection_view get_children() const
		{ return m_widgets.get_attributes(); }

		void theme_updated(main::config const& new_theme, main::widget_instance_info instance_info)
		{
			// TODO: Layouts must support a way to set outer margins to 0
			auto const& panel = instance_info.section_level%2 == 0?
				new_theme.main_panel :
				new_theme.other_panel;
			m_layout.margin_x = panel.padding;
			m_layout.margin_y = 0;
			m_null_texture = new_theme.misc_textures.null;
		}

 		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend)
		{
			// TODO: There will be multiple "group" widgets, and code duplication should be avoided
			auto const null_texture = m_null_texture->get_backend_resource(backend).get();
			return main::widget_layer_stack{
				.background = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				},
				.sel_bg_mask = main::widget_layer_mask{
					.offset = displacement{},
					.texture = null_texture,
				},
				.selection_background = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				},
				.foreground = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				},
				.frame = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				},
				.input_marker = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				}
			};
		}

	private:
		layouts::rowmajor_table m_layout{2};
		ControlWidget m_input_widget;
		text_to_float_input m_textbox;
		main::widget_collection m_widgets;
		main::widget_user_interaction_handler<float_input> m_on_value_changed{no_operation_tag{}};

		main::immutable_shared_texture m_null_texture;
	};
}

#endif