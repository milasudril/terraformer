#ifndef TERRAFORMER_UI_MAIN_GRAPHICS_RESOURCE_FACTORY_HPP
#define TERRAFORMER_UI_MAIN_GRAPHICS_RESOURCE_FACTORY_HPP

#include "./texture.hpp"

namespace terraformer::ui::main
{
	class graphics_resource_factory
	{
	public:
		template<class FactoryType>
		explicit graphics_resource_factory(FactoryType& factory):
			m_handle{&factory},
			m_vtable_pointer{&s_vtable<FactoryType>}
		{}

		texture create(std::type_identity<texture>, image const& src)
		{ return m_vtable_pointer->create_texture_from_image(m_handle, src);}

	private:
		struct vtable
		{
			texture (*create_texture_from_image)(void*, image const&);
		};

		template<class FactoryType>
		static constexpr vtable s_vtable{
			.create_texture_from_image = [](void* object, image const& img) {
				return static_cast<FactoryType*>(object)
					->create(std::type_identity<texture>{}, img);
			}
		};

		void* m_handle;
		vtable const* m_vtable_pointer;
	};
}

#endif