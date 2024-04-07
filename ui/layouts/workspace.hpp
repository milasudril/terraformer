#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP

#include "ui/main/widget_list.hpp"

namespace terraformer::ui::layout_handling
{
	template<class DefaultTexturePool>
	class workspace
	{
	public:
		explicit workspace(DefaultTexturePool&& texture_pool = DefaultTexturePool{}):
			m_textures{std::move(texture_pool)}
		{}

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

		auto const& background() const
		{
			return m_textures.main_panel_background;
		}

		auto const& foreground() const{ m_textures.null_texture; }

		template<class T>
 		bool handle_event(T const&) const
 		{
			return false;
		}

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{ return size; }

	private:
		main::widget_list<typename DefaultTexturePool::texture_type> m_widgets;
		DefaultTexturePool m_textures;
	};
}

#endif
