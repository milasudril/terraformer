//@	{"target":{"name":"table.o"}}

#include "./table.hpp"

void terraformer::ui::widgets::table::record::append_pending_widgets()
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

		auto const ref = i->second.object.get();
		auto const ptr = ref.get_pointer();
		auto const vt = ref.get_vtable();
		vt.append_to(ptr, m_parent);
	}
}

terraformer::ui::widgets::table::table::table(
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

	append(std::ref(m_dummy), main::widget_geometry{});
	for(auto& item : m_field_names)
	{ append(std::ref(item), main::widget_geometry{}); }
}

void terraformer::ui::widgets::table::toggle_record_visibility(std::u8string_view item)
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

void terraformer::ui::widgets::table::toggle_field_visibility(std::u8string_view item)
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
