#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP

#include "./type_erased_layout_controller_entry.hpp"

#include "lib/array_classes/multi_array.hpp"

namespace terraformer::ui::layout_handling
{
	class workspace
	{
	public:
		widget_dimensions get_dimensions(widget_dimensions dim) const
		{ return m_get_dimensions(m_object, dim); }

		template<layout_entry Entry>
		void attach(std::reference_wrapper<Entry const> entry)
		{
		m_entries.emplace_back(entry_geometry{}, type_erased_layout_controller_entry{entry});
		}

	private:
		widget_dimensions m_required_dimensions;

		struct entry_geometry
		{
			widget_dimensions dimensions;
			widget_location location;
		};

		multi_array<entry_geometry, type_erased_layout_controller_entry> m_entries;
	};
}

#endif