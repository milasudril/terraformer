#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP

#include "ui/main/widget_list.hpp"

#include <functional>

namespace terraformer::ui::layouts
{
	template<class ... Args>
	void do_show_widgets(Args&&... args)
	{ show_widgets(std::forward<Args>(args)...); }

	template<class StockTexturesRepo>
	class workspace
	{
	public:
		using drawing_surface_type = typename StockTexturesRepo::texture_type;

		explicit workspace(
			std::reference_wrapper<StockTexturesRepo const> texture_repo = StockTexturesRepo::get_default_instance()
		):
			m_textures{texture_repo}
		{}

		template<class ... Args>
		workspace& append(Args&&... args)
		{
			m_widgets.append(std::forward<Args>(args)...);
			return *this;
		}

		void render()
		{ render_widgets(m_widgets); }

		auto const& background() const
		{ return m_textures.get().main_panel_background; }

		auto const& foreground() const
		{ return m_textures.get().null_texture; }

		bool handle_event(wsapi::cursor_motion_event const& event)
		{
			auto i = find(event.where, m_widgets);
			if(i == m_widgets.npos)
			{ return false; }
#if 0
			// TODO: Need to add callback in widget list
			auto const widgets = m_widgets.widget_pointers();
			auto const cme_handlers = m_widgets.mouse_button_callbacks();

			return mbe_handlers[i](widgets[i], mbe);
#endif
			return false;
		}

		bool handle_event(wsapi::mouse_button_event const& event)
		{
			auto i = find(event.where, m_widgets);
			if(i == m_widgets.npos)
			{ return false; }

			auto const widgets = m_widgets.widget_pointers();
			auto const mbe_handlers = m_widgets.mouse_button_callbacks();

			return mbe_handlers[i](widgets[i], event);
		}

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{
			auto const size_callbacks = m_widgets.size_callbacks();
			auto const widget_pointers = m_widgets.widget_pointers();
			auto const widget_visibilities = m_widgets.widget_visibilities();
			auto const widget_geometries = m_widgets.widget_geometries();
			auto const n = std::size(m_widgets);
			for(auto k = m_widgets.first_element_index(); k != n; ++k)
			{
				// TODO: Maybe size needs to be propagated to invisible widgets
				if(widget_visibilities[k] == main::widget_visibility::visible) [[likely]]
				{
					size_callbacks[k](
						widget_pointers[k],
						wsapi::fb_size {
							.width = static_cast<int>(widget_geometries[k].size[0]),
							.height = static_cast<int>(widget_geometries[k].size[1])
						}
					);
				}
			}
			return size;
		}

		template<class Renderer>
		void show_widgets(Renderer&& renderer)
		{ do_show_widgets(std::forward<Renderer>(renderer), m_widgets); }

	private:

		main::widget_list<drawing_surface_type const&> m_widgets;
		std::reference_wrapper<StockTexturesRepo const> m_textures;
	};
}

#endif
