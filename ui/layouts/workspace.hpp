#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP

#include "ui/main/widget_list.hpp"

namespace terraformer::ui::layout_handling
{
	template<class DrawingSurface>
	class workspace
	{
	public:
		template<class ... Args>
		workspace& append(Args&&... args)
		{ 
			m_widgets.append(std::forward<Args>(args)...);
			return *this;
		}

	private:
		main::widget_list<DrawingSurface> m_widgets;
	};
}

#endif
