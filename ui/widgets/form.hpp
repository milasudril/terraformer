#ifndef TERRAFORMER_UI_WIDGETS_FORM_HPP
#define TERRAFORMER_UI_WIDGETS_FORM_HPP

#include "./label.hpp"

#include "ui/layouts/table.hpp"
#include "ui/widgets/widget_group.hpp"

#include "lib/common/string_to_value_map.hpp"

namespace terraformer::ui::widgets
{
	class form:private widget_group<layouts::table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::prepare_for_presentation;
		using widget_group::theme_updated;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;
		using widget_group::confirm_size;
		using widget_group::set_refresh_function;
		using widget_group::refresh;

		explicit form(
			iterator_invalidation_handler_ref iihr,
			main::widget_orientation orientation = main::widget_orientation::vertical
		):
			widget_group{
				iihr,
				orientation == main::widget_orientation::vertical?
					layouts::table{
						layouts::table::columns(
							layouts::table::cell_size::use_default{},
							layouts::table::cell_size::expand{}
						)
					}
					:layouts::table{
						layouts::table::rows(
							layouts::table::cell_size::use_default{},
							layouts::table::cell_size::expand{}
						)
					}
			},
			m_orientation{orientation}
		{ is_transparent = false; }

		template<class Function>
		form& on_content_updated(Function&& func)
		{
			m_on_content_updated = std::forward<Function>(func);
			return *this;
		};

		template<class FieldDescriptor, class ... InputWidgetParams>
		auto& create_widget(FieldDescriptor const& field, InputWidgetParams&&... input_widget_params)
		{
			using input_widget_type = typename FieldDescriptor::input_widget_type;

			bool label_on_input_field = false;
			if constexpr(requires(){{field.label_on_input_field};})
			{
				static_assert(requires(input_widget_type& widget){{widget.text(field.label)};});
				label_on_input_field = field.label_on_input_field;

				// TODO: Span record so input field replaces label
				auto field_label = std::make_unique<label>();
				append(std::ref(*field_label) ,ui::main::widget_geometry{});
				m_widgets.push_back(resource{std::move(field_label)});
			}
			else
			{
				auto field_label = std::make_unique<interactive_label>();
				field_label->on_activated([this, label = std::u8string{field.label}](auto&& ...){
					toggle_field_value_visibility(label);
				});

				field_label->text(field.label);
				append(std::ref(*field_label) ,ui::main::widget_geometry{});
				m_widgets.push_back(resource{std::move(field_label)});
			}

			auto field_input_widget = []<class ... T>(
				iterator_invalidation_handler_ref iihr,
				main::widget_orientation orientation,
				T&&... input_widget_params
			){
				if constexpr(std::is_constructible_v<input_widget_type, iterator_invalidation_handler_ref, main::widget_orientation, std::remove_cvref_t<T>...>)
				{
					if constexpr(sizeof...(InputWidgetParams))
					{
						if constexpr(std::is_same_v<std::remove_cvref_t<pick_first_t<InputWidgetParams...>>, main::widget_orientation>)
						{ return std::make_unique<input_widget_type>(iihr, std::forward<T>(input_widget_params)...); }
					}
					return std::make_unique<input_widget_type>(iihr, orientation, std::forward<T>(input_widget_params)...);
				}
				else
				if constexpr(std::is_constructible_v<input_widget_type, iterator_invalidation_handler_ref, std::remove_cvref_t<T>...>)
				{ return std::make_unique<input_widget_type>(iihr, std::forward<T>(input_widget_params)...); }
				else
				{ return std::make_unique<input_widget_type>(std::forward<T>(input_widget_params)...); }
			}(
				iterator_invalidation_handler(),
				m_orientation == main::widget_orientation::vertical?
					main::widget_orientation::horizontal:
					main::widget_orientation::vertical,
				std::forward<InputWidgetParams>(input_widget_params)...
			);

			auto& ret = *field_input_widget;

			if constexpr(requires(input_widget_type& widget){{widget.text(field.label)};})
			{
				if(label_on_input_field)
				{ ret.text(field.label); }
			}

			if constexpr(requires(input_widget_type const& widget){{widget.value()};})
			{
				ret.value(field.value_reference.get());
				ret.on_value_changed([value = field.value_reference, this]<class ... Args>(auto& widget, Args&&... args){
					using widget_value_type = std::remove_cvref_t<decltype(widget.value())>;
					auto&& new_val = widget.value();
					if constexpr(requires(widget_value_type const& val){{FieldDescriptor::is_value_valid(val)};})
					{
						if(!FieldDescriptor::is_value_valid(widget.value()))
						{
							widget.value(static_cast<widget_value_type>(value.get()));
							return;
						}
					}
					value.get() = std::forward<widget_value_type>(new_val);
					m_on_content_updated(*this, std::forward<Args>(args)...);
				});
			}
			else
			if constexpr(requires(input_widget_type& obj, main::widget_user_interaction_handler<input_widget_type>&& callback){
				{obj.on_content_updated(std::move(callback))};
			})
			{
				ret.on_content_updated([this]<class ... Args>(auto&, Args&&... args) {
					m_on_content_updated(*this, std::forward<Args>(args)...);
				});
			}

			append(std::ref(ret), ui::main::widget_geometry{});
			m_widgets.push_back(resource{std::move(field_input_widget)});

			auto const widget_attributes = get_children();
			auto const last_element = widget_attributes.element_indices().back();

			if constexpr(requires(FieldDescriptor const& f){{f.expand_layout_cell} -> std::convertible_to<bool>;})
			{
				if(field.expand_layout_cell)
				{
					auto const widget_states = widget_attributes.widget_states();
					layout.set_cell_size(last_element.get(), layouts::table::cell_size::expand{});
					widget_states[last_element].maximized = true;
				}
			}

			if constexpr(requires(FieldDescriptor const& f){{f.maximize_widget} -> std::convertible_to<bool>;})
			{
				auto const widget_states = widget_attributes.widget_states();
				widget_states[last_element].maximized = field.maximize_widget;
			}

			m_fields.insert(std::pair{field.label, last_element});

			++m_record_count;
			return ret;
		}

		void toggle_field_value_visibility(std::u8string_view field_name)
		{
			auto const i = m_fields.find(field_name);
			if(i == std::end(m_fields))
			{ return; }

			auto const widget_attributes = get_children();
			auto const widget_states = widget_attributes.widget_states();
			widget_states[i->second].collapsed = !widget_states[i->second].collapsed;
		}

		void append_pending_widgets()
		{}

	private:
		struct vtable
		{
			template<class StoredType>
			constexpr vtable(std::type_identity<StoredType>){}
		};
		using resource = unique_resource<vtable>;

		using user_interaction_handler = main::widget_user_interaction_handler<form>;
		user_interaction_handler m_on_content_updated{no_operation_tag{}};

		single_array<unique_resource<vtable>> m_widgets;
		main::widget_orientation m_orientation;

		size_t m_record_count = 0;

		u8string_to_value_map<main::widget_collection_ref::index_type> m_fields;
	};
}

#endif
