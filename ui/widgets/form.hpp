//@	{"dependencies_extra":[{"ref":"./button.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_FORM_HPP
#define TERRAFORMER_UI_WIDGETS_FORM_HPP

#include "./label.hpp"
#include "ui/layouts/rowmajor_table.hpp"

namespace terraformer::ui::widgets
{
	class form:private widget_group<layouts::rowmajor_table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::prepare_for_presentation;
		using widget_group::theme_updated;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;

		explicit form(main::widget_orientation orientation = main::widget_orientation::horizontal):
			widget_group{
				orientation == main::widget_orientation::horizontal?
					static_cast<size_t>(2) :
					static_cast<size_t>(1)
			}
		{ is_transparent = false; }

		template<class FieldDescriptor>
		auto& create_widget(FieldDescriptor&& field)
		{
			auto field_label = std::make_unique<label>();
			field_label->text(field.label);
			append(std::ref(*field_label) ,ui::main::widget_geometry{});
			m_widgets.push_back(resource{std::move(field_label)});

			auto field_input_widget = field.create_widget();
			auto& ret = *field_input_widget;
			ret.on_value_changed(std::move(field.on_value_changed));
			append(std::ref(ret), ui::main::widget_geometry{});
			m_widgets.push_back(resource{std::move(field_input_widget)});
			return ret;
		}

	private:
		struct vtable
		{
			template<class StoredType>
			constexpr vtable(std::type_identity<StoredType>){}
		};
		using resource = unique_resource<vtable>;
		single_array<unique_resource<vtable>> m_widgets;
	};
}

#endif
