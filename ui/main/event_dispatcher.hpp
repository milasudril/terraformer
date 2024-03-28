#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "ui/drawing_api/gl_texture.hpp"
#include "ui/wsapi/native_window.hpp"
#include "lib/array_classes/multi_array.hpp"

namespace terraformer::ui::main
{
	template<class T>
	concept widget = requires(
		T& obj,
		wsapi::fb_size size,
		wsapi::cursor_position pos,
		wsapi::mouse_button_event const& mbe,
		drawing_api::gl_texture& texture
	)
	{
		{ std::as_const(obj).render_to(texture) } -> std::same_as<void>;
		{ obj.handle_event(std::as_const(pos)) } -> std::same_as<bool>;
		{ obj.handle_event(mbe) } -> std::same_as<bool>;
		{ std::as_const(obj).handle_event(std::as_const(size)) } -> std::same_as<wsapi::fb_size>;
	};

	template<class T>
	concept widget_size_policy = requires(T const& obj, size_t k, wsapi::fb_size size)
	{
		{ obj(std::as_const(k), std::as_const(size)) } -> std::same_as<wsapi::fb_size>;
	};

	template<class T>
	concept widget_hit_policy = requires(T const& obj, size_t k, wsapi::cursor_position pos)
	{
		{ obj(std::as_const(k), std::as_const(pos)) } -> std::same_as<bool>;
	};

	class event_dispatcher
	{
	public:
		template<widget Widget>
		event_dispatcher& append(std::reference_wrapper<Widget> w)
		{
			m_objects.push_back(
				std::in_place_t{},
				&w,
				drawing_api::gl_texture{},
				[](void const* obj, drawing_api::gl_texture& texture){
					return static_cast<Widget const*>(obj)->render_to(texture);
				},
				[](void* obj, wsapi::cursor_position pos){
					return static_cast<Widget*>(obj)->handle_event(pos);
				},
				[](void* obj, wsapi::mouse_button_event const& mbe){
					return static_cast<Widget*>(obj)->handle_event(mbe);
				},
				[](void const* obj, wsapi::fb_size size){
					return static_cast<Widget const*>(obj)->handle_event(size);
				}
			);

			return *this;
		}

		void render_widgets()
		{
			auto const objects = m_objects.get<0>();
			auto const textures = m_objects.get<1>();
			auto const dispatchers = m_objects.get<2>();
			auto const n_objects = std::size(m_objects);
			for(auto k = m_objects.first_element_index();
				k != n_objects;
				++k
			)
			{ dispatchers[k](std::as_const(objects[k]), textures[k]); }
		}

		template<widget_hit_policy WidgetHitPolicy>
		bool dispatch(wsapi::cursor_position pos, WidgetHitPolicy&& hit) const
		{
			auto const objects = m_objects.get<0>();
			auto const dispatchers = m_objects.get<3>();
			auto const n_objects = std::size(m_objects);
			for(auto k = m_objects.first_element_index();
				k != n_objects;
				++k
			)
			{
				if(hit(k, pos) && dispatchers[k](objects[k], pos))
				{ return true; }
			}
			return false;
		}

		bool dispatch(wsapi::mouse_button_event const& mbe)
		{
			auto const objects = m_objects.get<0>();
			auto const dispatchers = m_objects.get<4>();
			auto const n_objects = std::size(m_objects);
			for(auto k = m_objects.first_element_index();
				k != n_objects;
				++k
			)
			{
				if(dispatchers[k](objects[k], mbe))
				{ return true; }
			}
			return false;
		}

		template<widget_size_policy SizePolicy>
		void dispatch(wsapi::fb_size size, SizePolicy&& widget_size)
		{
			auto const objects = m_objects.get<0>();
			auto const textures = m_objects.get<1>();
			auto const dispatchers = m_objects.get<5>();
			auto const n_objects = std::size(m_objects);
			for(auto k = m_objects.first_element_index();
				k != n_objects;
				++k
			)
			{
				auto const new_size = widget_size(k, dispatchers[k](objects[k], size));
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