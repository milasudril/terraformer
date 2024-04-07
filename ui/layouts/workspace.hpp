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

		void render()
		{
			render_widgets(m_widgets);
		}

 		DrawingSurface background() const{}

 		DrawingSurface foreground() const{}

		template<class T>
 		bool handle_event(T const&) const
 		{
			return false;
		}

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{ return size; }

	private:
		main::widget_list<DrawingSurface> m_widgets;
	};

	static_assert(main::widget<workspace<int>, int>);
}

#endif
