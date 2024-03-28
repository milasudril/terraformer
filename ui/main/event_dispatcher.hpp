#ifndef TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP
#define TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP

#include "ui/drawing_api/gl_texture.hpp"
#include "ui/wsapi/native_window.hpp"
#include "lib/array_classes/multi_array.hpp"

namespace terraformer::ui::main
{
	template<class T>
	concept event_dispatcher = requires(
		T& obj,
		wsapi::fb_size size,
		wsapi::cursor_position pos,
		wsapi::mouse_button_event const& mbe,
		drawing_api::gl_texture& texture
	)
	{
		{ obj.render(texture) } -> std::same_as<void>;
		{ obj.dispatch(std::as_const(pos)) } -> std::same_as<bool>;
		{ obj.dispatch(mbe) } -> std::same_as<bool>;
		{ std::as_const(obj).dispatch(std::as_const(size)) } -> std::same_as<void>;
	};

	class entity_list
	{
	public:
		void render()
		{
			auto const objects = m_objects.get<0>();
			auto const textures = m_objects.get<1>();
			auto const dispatchers = m_objects.get<2>();
			for(auto k = m_objects.first_element_index();
				k != std::size(m_objects);
				++k
			)
			{ dispatchers[k](std::as_const(objects[k]), textures[k]); }
		}

		bool dispatch(wsapi::cursor_position pos) const
		{
			auto const objects = m_objects.get<0>();
			auto const dispatchers = m_objects.get<3>();
			for(auto k = m_objects.first_element_index();
				k != std::size(m_objects);
				++k
			)
			{
				if(dispatchers[k](objects[k], pos))
				{ return true; }
			}
			return false;
		}

		bool dispatch(wsapi::mouse_button_event const& mbe)
		{
			auto const objects = m_objects.get<0>();
			auto const dispatchers = m_objects.get<4>();
			for(auto k = m_objects.first_element_index();
				k != std::size(m_objects);
				++k
			)
			{
				if(dispatchers[k](objects[k], mbe))
				{ return true; }
			}
			return false;
		}

		void dispatch(wsapi::fb_size size)
		{
			auto const objects = m_objects.get<0>();
			auto const textures = m_objects.get<1>();
			auto const dispatchers = m_objects.get<5>();
			for(auto k = m_objects.first_element_index();
				k != std::size(m_objects);
				++k
			)
			{
				auto const new_size = dispatchers[k](objects[k], size);
				auto texture_descriptor = textures[k].descriptor();
				texture_descriptor.width = new_size.width;
				texture_descriptor.height = new_size.height;
				textures[k].set_format(texture_descriptor);
			}
		}

	private:
		using render_dispatcher = void (*)(void const*, drawing_api::gl_texture& texture);
		using cursor_position_dispatcher = bool (*)(void*, wsapi::cursor_position);
		using mouse_button_event_dispatcher = bool (*)(void*, wsapi::mouse_button_event const& mbe);
		using size_event_dispatcher = wsapi::fb_size (*)(void const*, wsapi::fb_size);

		multi_array<
			void*,
			drawing_api::gl_texture,
			render_dispatcher,
			cursor_position_dispatcher,
			mouse_button_event_dispatcher,
			size_event_dispatcher
		> m_objects;
	};
}

#endif