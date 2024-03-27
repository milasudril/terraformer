#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_TYPE_ERASED_LAYOUT_CONTROLLER_ENTRY_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_TYPE_ERASED_LAYOUT_CONTROLLER_ENTRY_HPP

#include "./layout_controller_entry.hpp"

namespace terraformer::ui::layout_handling
{
	class type_erased_layout_controller_entry
	{
	public:
		template<layout_controller_entry Entry>
		explicit type_erased_layout_controller_entry(std::reference_wrapper<Entry const> object):
			m_object{&object.get()},
			m_get_dimensions{[](void const* object, widget_dimensions dim){
				return static_cast<Entry const*>(m_object)->get_dimensions(dim);
			}}
		{}

		widget_dimensions get_dimensions(widget_dimensions dim) const
		{ return m_get_dimensions(m_object, dim); }

	private:
		void const* m_object;
		widget_dimensions (*)(m_get_dimensions)(void const*, widget_dimensions);
	};
}

#endif