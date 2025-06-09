#ifndef TERRAFORMER_UI_WIDGETS_TABLE_HPP
#define TERRAFORMER_UI_WIDGETS_TABLE_HPP

#include "./label.hpp"

#include "ui/layouts/table.hpp"
#include "ui/widgets/widget_group.hpp"

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

			for(auto& item : m_field_names)
			{ append(std::ref(item), main::widget_geometry{}); }
		}

	private:
		single_array<label> m_field_names;
		main::widget_orientation m_orientation;
	};

	static_assert(
		std::is_constructible_v<
			table,
			main::widget_collection::iterator_invalidation_handler_ref, main::widget_orientation, std::array<const char8_t*, 1>
		>
	);
}

#endif
