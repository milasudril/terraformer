#ifndef TERRAFORMER_UI_MAIN_KEYBOARD_NAVIGATION_CONTROLLER_HPP
#define TERRAFORMER_UI_MAIN_KEYBOARD_NAVIGATION_CONTROLLER_HPP

#include "./widget_reference.hpp"

namespace terraformer::ui::main
{
	class keyboard_navigation_controller
	{
	public:
		keyboard_navigation_controller() = default;

		explicit keyboard_navigation_controller(
			flat_widget_collection_view::widget_span widgets,
			int search_direction
		):
			m_widgets{widgets}
		{ step(search_direction); }

		void step(int dir)
		{
			auto next_index = m_index;
			auto const widget_states = m_widgets.get_by_type<std::reference_wrapper<widget_state const>>();
			for(auto k = m_widgets.first_element_index(); k != std::size(m_widgets); ++k)
			{
				next_index += dir;
				if(next_index == flat_widget_collection_view::npos)
				{ next_index = m_widgets.last_element_index(); }
				if(next_index == std::size(m_widgets))
				{ next_index = m_widgets.first_element_index(); }

				if(widget_states[k].get().accepts_keyboard_input())
				{ break; }
			}

			if(next_index != m_index)
			{
				/*
				if(!try_dispatch(m_widgets, m_index, keyboard_focus_leave_event{}))
				{ return; }

				if(!try_dispatch(m_widgets, next_index, keyboard_focus_enter_event{}))
				{ return; }
				*/

				m_index = next_index;
			}
		}

		auto current_index() const
		{ return m_index; }

		auto widgets() const
		{ return m_widgets; }

	private:
		flat_widget_collection_view::widget_span m_widgets;
		flat_widget_collection_view::index_type m_index{flat_widget_collection_view::npos};
	};
}

#endif