#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP

#include "ui/main/widget_list.hpp"

namespace terraformer::ui::layout_handling
{
	template<class RenderSurface>
	class workspace
	{
	public:


	private:
		main::widget_list<RenderSurface> m_widgets;
	};
}

#endif