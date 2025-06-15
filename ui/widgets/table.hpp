#ifndef TERRAFORMER_UI_WIDGETS_TABLE_HPP
#define TERRAFORMER_UI_WIDGETS_TABLE_HPP

#include "./label.hpp"

#include "lib/common/move_only_function.hpp"
#include "lib/common/string_to_value_map.hpp"
#include "ui/layouts/table.hpp"
#include "ui/main/widget_collection.hpp"
#include "ui/main/widget_geometry.hpp"
#include "ui/widgets/widget_group.hpp"
#include <stdexcept>
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

			record(record const&) = delete;
			record(record&&) = delete;
			record& operator=(record const&) = delete;
			record& operator=(record&&) = delete;
			~record() = default;

			explicit record(std::u8string_view id, table& parent): m_parent{parent}
			{
				m_id_label
					.on_activated([label = std::u8string{id}, parent = m_parent](auto&&...){
						parent.get().toggle_record_visibility(label);
					})
					.text(id);
			}

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
						call_on_content_updated(std::forward<Args>(args)...);
					});
				}
				else
				if constexpr(requires(input_widget_type& obj, main::widget_user_interaction_handler<input_widget_type>&& callback){
					{obj.on_content_updated(std::move(callback))};
				})
				{
					ret.on_content_updated([this]<class ... Args>(auto&, Args&&... args) {
						call_on_content_updated(std::forward<Args>(args)...);
					});
				}
				m_widgets.emplace(field.label, widget{std::move(field_input_widget)});

				return ret;
			}

			void append_pending_widgets()
			{
				auto const& field_names =  m_parent.get().field_names();
				if(std::size(m_widgets) != std::size(field_names).get())
				{ throw std::runtime_error{"Table is missing fields"}; }

				m_parent.get().append(std::ref(m_id_label), main::widget_geometry{});
				for(auto& item :field_names)
				{
					auto const i = m_widgets.find(item.value());
					if(i == std::end(m_widgets)) [[unlikely]]
					{
						auto const string_begin = reinterpret_cast<char const*>(item.value().data());
						auto const string_end = string_begin + item.value().size();
						throw std::out_of_range{std::string{string_begin, string_end}};
					}

					auto const ref = i->second.get();
					auto const ptr = ref.get_pointer();
					auto const vt = ref.get_vtable();
					vt.append_to(ptr, m_parent);
				}
			}

		private:
			interactive_label m_id_label;
			u8string_to_value_map<widget> m_widgets;
			std::reference_wrapper<table> m_parent;

			using user_interaction_handler = main::widget_user_interaction_handler<record>;
		user_interaction_handler m_on_content_updated{};

			template<class ... Args>
			void call_on_content_updated(Args&&... args)
			{
				if(m_on_content_updated == user_interaction_handler{})
				{ m_parent.get().m_on_content_updated(m_parent.get(), std::forward<Args>(args)...); }
				else
				{ m_on_content_updated(*this, std::forward<Args>(args)...); }
			}
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
					 layouts::table{layouts::table::column_count{std::size(field_names).get() + 1}}
					:layouts::table{layouts::table::row_count{std::size(field_names).get() + 1}}
			},
			m_orientation{orientation}
		{
			is_transparent = false;

			for(auto item : field_names)
			{
				interactive_label header;
				header
					.on_activated([field_name = std::u8string{item}, this](auto&&...){
						toggle_field_visibility(field_name);
					})
					.text(item);
				m_field_names.push_back(std::move(header));
			}

			append_field_names();
		}

		template<class RecordDescriptor>
		record& create_widget(RecordDescriptor&& descriptor)
		{
			auto i = m_records.try_emplace(std::u8string{descriptor.label}, descriptor.label, *this);
			m_record_indices.try_emplace(
				std::u8string{descriptor.label},
				get_attributes().element_indices().back() + 1
			);
			return i.first->second;
		}

		template<class Function>
		table& on_content_updated(Function&& func)
		{
			m_on_content_updated = std::forward<Function>(func);
			return *this;
		};

		single_array<interactive_label> const& field_names() const
		{ return m_field_names; }

		void toggle_record_visibility(std::u8string_view item)
		{
			if(m_field_names.empty())
			{ return; }

			auto const i = m_record_indices.find(item);
			if(i == std::end(m_record_indices))
			{
				auto const string_begin = reinterpret_cast<char const*>(item.data());
				auto const string_end = string_begin + item.size();
				throw std::out_of_range{std::string{string_begin, string_end}};
			}

			auto const attributes = get_attributes();
			auto const states = attributes.widget_states();
			for(auto k = i->second + 1; k != i->second + 1 + std::size(m_field_names).get(); ++k)
			{ states[k].collapsed = !states[k].collapsed; }
		}

		void toggle_field_visibility(std::u8string_view item)
		{
			auto i = std::ranges::find_if(m_field_names, [look_for = item](auto const& item){
				return item.value() == look_for;
			});
			if(i == std::end(m_field_names))
			{ return; }

			auto const index = i - std::begin(m_field_names);
			auto const colcount = std::size(m_field_names).get() + 1;
			auto const start_offset = index + colcount + 1;
			auto const attributes = get_attributes();
			auto const widget_count = std::size(attributes);
			auto const states = attributes.widget_states();
			for(auto k = attributes.element_indices().front() + start_offset; k < widget_count; k += colcount)
			{ states[k].collapsed = !states[k].collapsed; }
		}

	private:
		label m_dummy;
		single_array<interactive_label> m_field_names;
		main::widget_orientation m_orientation;
		u8string_to_value_map<record> m_records;
		u8string_to_value_map<main::widget_collection::index_type> m_record_indices;

		void append_field_names()
		{
			append(std::ref(m_dummy), main::widget_geometry{});
			for(auto& item : m_field_names)
			{ append(std::ref(item), main::widget_geometry{}); }
		}

		using user_interaction_handler = main::widget_user_interaction_handler<table>;
		user_interaction_handler m_on_content_updated{no_operation_tag{}};
	};

	static_assert(
		std::is_constructible_v<
			table,
			main::widget_collection::iterator_invalidation_handler_ref, main::widget_orientation, std::array<const char8_t*, 1>
		>
	);
}

#endif
