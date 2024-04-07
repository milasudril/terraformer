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


		bool handle_event(wsapi::cursor_position pos)
		{
			auto i = find(pos, m_widgets);
			if(i == m_widgets.npos)
			{ return false; }

			printf("Cursor over widget %zu\n", i.get());
			fflush(stdout);

			return i->handle_event(pos);
		}

		bool handle_event(wsapi::mouse_button_event const& mbe)
		{
			auto i = find(mbe.where, m_widgets);
			if(i == m_widgets.npos)
			{ return false; }

			printf("Cursor over widget %zu\n", i.get());
			fflush(stdout);

			return i->handle_event(mbe);
		}

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{ return size; }

		template<class Renderer>
		void show_widgets(Renderer&& renderer)
		{ do_show_widgets(std::forward<Renderer>(renderer), m_widgets); }

	private:

		main::widget_list<drawing_surface_type> m_widgets;
		std::reference_wrapper<StockTexturesRepo const> m_textures;
	};
}

#endif
