#ifndef TERRAFORMER_UI_MAIN_GRAPHICS_BACKEND_HPP
#define TERRAFORMER_UI_MAIN_GRAPHICS_BACKEND_HPP

#include "./generic_texture.hpp"
#include "lib/common/global_instance_counter.hpp"
#include "lib/pixel_store/image.hpp"

#include <type_traits>

namespace terraformer::ui::main
{
	class graphics_backend:public global_instance_counter
	{
	public:
		template<class BackendType>
		explicit graphics_backend(BackendType& backend):
			m_handle{&backend},
			m_vptr{&vt<BackendType>}
		{}

		generic_unique_texture create(std::type_identity<generic_unique_texture>, image const& img)
		{ return m_vptr->create_texture_from_image(m_handle, get_global_id(), img); }

	private:
		struct vtable
		{
			generic_unique_texture (*create_texture_from_image)(void*, uint64_t, image const&);
		};

		template<class T>
		static constexpr vtable vt{
			.create_texture_from_image = [](void* handle, uint64_t backend_id, image const& img) {
					return static_cast<T*>(handle)->create(std::type_identity<generic_unique_texture>{}, backend_id, img);
			}
		};

		void* m_handle;
		vtable const* m_vptr;
	};
}

#endif