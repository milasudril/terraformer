#ifndef TERRAFORMER_UI_WIDGETS_TABLE_HPP
#define TERRAFORMER_UI_WIDGETS_TABLE_HPP

#include "./label.hpp"

#include "ui/layouts/table.hpp"
#include "ui/main/widget_collection.hpp"
#include "ui/main/widget_geometry.hpp"
#include "ui/widgets/widget_group.hpp"
#include <unistd.h>

namespace terraformer::ui::widgets
{
	class table:private widget_group<layouts::table>
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

		class record
		{
		public:
			struct widget_vtable
			{
				template<class StoredType>
				constexpr widget_vtable(std::type_identity<StoredType>):
					append_to{[](void* object, main::widget_collection& collection){
						collection.append(std::ref(*static_cast<StoredType*>(object)), main::widget_geometry{});
					}
				}
				{}
				void (*append_to)(void* object, main::widget_collection& collection);
			};
			using widget = unique_resource<widget_vtable>;

			explicit record(std::u8string_view, table& parent): m_parent{parent}
			{}

			span<widget const> widgets() const
			{ return m_widgets; }

			template<class Function>
			record& on_content_updated(Function&& func)
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
					m_parent.get().iterator_invalidation_handler(),
					m_parent.get().m_orientation == main::widget_orientation::vertical?
						main::widget_orientation::horizontal:
						main::widget_orientation::vertical,
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
				m_widgets.push_back(widget{std::move(field_input_widget)});

				return ret;
			}

			void append_pending_widgets()
			{
				for(auto& widget :m_widgets)
				{
					auto const ref = widget.get();
					auto const ptr = ref.get_pointer();
					auto const vt = ref.get_vtable();
					vt.append_to(ptr, m_parent);
				}
			}

		private:
			single_array<widget> m_widgets;
			std::reference_wrapper<table> m_parent;

			using user_interaction_handler = main::widget_user_interaction_handler<record>;
		user_interaction_handler m_on_content_updated{no_operation_tag{}};
		};

		template<size_t N>
		explicit table(
			iterator_invalidation_handler_ref iihr,
			main::widget_orientation orientation,
			std::array<char8_t const*, N> const& field_names
		):table{iihr, orientation, span{std::begin(field_names), std::end(field_names)}}
		{}

		explicit table(
			iterator_invalidation_handler_ref iihr,
			main::widget_orientation orientation,
			span<char8_t const* const> field_names
		):
			widget_group{
				iihr,
				orientation == main::widget_orientation::vertical?
					 layouts::table{layouts::table::column_count{std::size(field_names).get()}}
					:layouts::table{layouts::table::row_count{std::size(field_names).get()}}
			},
			m_orientation{orientation}
		{
			is_transparent = false;
			for(auto item : field_names)
			{ m_field_names.push_back(std::move(label{}.text(item))); }

			append_field_names();
		}

		template<class RecordDescriptor>
		record& create_widget(RecordDescriptor&& descriptor)
		{
			auto i = m_records.emplace(descriptor.label, record{descriptor.label, *this});
			return i.first->second;
		}

	private:
		single_array<label> m_field_names;
		main::widget_orientation m_orientation;
		std::unordered_map<std::u8string, record> m_records;

		void append_field_names()
		{
			for(auto& item : m_field_names)
			{ append(std::ref(item), main::widget_geometry{}); }
		}
	};

	static_assert(
		std::is_constructible_v<
			table,
			main::widget_collection::iterator_invalidation_handler_ref, main::widget_orientation, std::array<const char8_t*, 1>
		>
	);
}

#endif
