#ifndef TERRAFORMER_UI_MAIN_UI_CONTROLLER_HPP
#define TERRAFORMER_UI_MAIN_UI_CONTROLLER_HPP

#include "./config.hpp"

namespace terraformer::ui::main
{
	struct ui_controller_vtable
	{
		void (*theme_updated)(void*, config&&);
	};

	template<class T>
	constexpr ui_controller_vtable ui_controller_vtable_v{
		.theme_updated = [](void* handle, config&& new_config){
			static_cast<T*>(handle)->theme_updated(std::move(new_config));
		}
	};

	class ui_controller
	{
	public:
		template<class T>
		explicit ui_controller(T& controller):
			m_vtable{&ui_controller_vtable_v<T>},
			m_handle{&controller}
		{}

		void theme_updated(config&& new_config)
		{ m_vtable->theme_updated(m_handle, std::move(new_config)); }

	private:
		ui_controller_vtable const* m_vtable;
		void* m_handle;
	};
}

#endif