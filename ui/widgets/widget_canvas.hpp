#ifndef TERRAFORMER_UI_WIDGETS_WIDGET_CANVAS_HPP
#define TERRAFORMER_UI_WIDGETS_WIDGET_CANVAS_HPP

#include "./label.hpp"

#include "ui/layouts/none.hpp"
#include "ui/widgets/widget_group.hpp"

namespace terraformer::ui::widgets
{
	class widget_canvas:private widget_group<layouts::none>
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

		explicit widget_canvas(iterator_invalidation_handler_ref iihr):
			widget_group{
				iihr,
				layouts::none{layouts::none::cell_size_mode::expand}
			}
		{ is_transparent = false; }

		template<class Function>
		widget_canvas& on_content_updated(Function&& func)
		{
			m_on_content_updated = std::forward<Function>(func);
			return *this;
		};

		template<class FieldDescriptor, class ... InputWidgetParams>
		auto& create_widget(FieldDescriptor const& field, InputWidgetParams&&... input_widget_params)
		{
			using input_widget_type = typename FieldDescriptor::input_widget_type;
			auto field_input_widget = []<class ... T>(
				iterator_invalidation_handler_ref iihr,
				T&&... input_widget_params
			){
				if constexpr(std::is_constructible_v<input_widget_type, iterator_invalidation_handler_ref, T...>)
				{ return std::make_unique<input_widget_type>(iihr, std::forward<T>(input_widget_params)...); }
				else
				{ return std::make_unique<input_widget_type>(std::forward<T>(input_widget_params)...); }
			}(
				iterator_invalidation_handler(),
				std::forward<InputWidgetParams>(input_widget_params)...
			);
			auto& ret = *field_input_widget;
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
							widget.value(value);
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

			if constexpr(requires(FieldDescriptor const& f){{f.expand_widget} -> std::convertible_to<bool>;})
			{
				if(field.expand_widget)
				{
					auto const widget_attributes = get_children();
					auto const last_element = widget_attributes.element_indices().back();
					auto const widget_states = widget_attributes.widget_states();
					widget_states[last_element].maximized = true;
				}
			}

			return ret;
		}

	private:
		struct vnone
		{
			template<class StoredType>
			constexpr vnone(std::type_identity<StoredType>){}
		};
		using resource = unique_resource<vnone>;

		using user_interaction_handler = main::widget_user_interaction_handler<widget_canvas>;
		user_interaction_handler m_on_content_updated{no_operation_tag{}};

		single_array<unique_resource<vnone>> m_widgets;
	};
}

#endif
