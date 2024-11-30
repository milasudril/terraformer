#ifndef TERRAFORMER_UI_MAIN_WINDOW_REF_HPP
#define TERRAFORMER_UI_MAIN_WINDOW_REF_HPP

#include <functional>
#include <string>

namespace terraformer::ui::main
{
	struct window_ref_vtable
	{
		void (*set_title)(void* handle, char const* new_title);
		std::u8string (*get_clipboard_string)(void* handle);
		void (*set_clipboard_string)(void* handle, char8_t const* str);
	};

	template<class Window>
	struct window_traits;

	template<class Window>
	constexpr window_ref_vtable window_vtable{
		.set_title = [](void* handle, char const* new_title) {
			window_traits<Window>::set_title(*static_cast<Window*>(handle), new_title);
		},
		.get_clipboard_string = [](void* handle){
			return window_traits<Window>::get_clipboard_string(*static_cast<Window*>(handle));
		},
		.set_clipboard_string = [](void* handle, char8_t const* str){
			window_traits<Window>::set_clipboard_string(*static_cast<Window*>(handle), str);
		},
	};

	class window_ref
	{
	public:
		template<class T>
		explicit window_ref(T& window):
			m_vtable{&window_vtable<T>},
			m_handle{&window}
		{}

		void set_title(char const* new_title)
		{ m_vtable->set_title(m_handle, new_title); }

		void* handle() const
		{ return m_handle; }

		std::u8string get_clipboard_string() const
		{ return m_vtable->get_clipboard_string(m_handle); }

		void set_clipboard_string(char8_t* str) const
		{ m_vtable->set_clipboard_string(m_handle, str); }

	private:
		struct window_ref_vtable const* m_vtable;
		void* m_handle;
	};
}

#endif