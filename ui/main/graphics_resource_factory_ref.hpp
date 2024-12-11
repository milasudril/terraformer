#ifndef TERRAFORMER_UI_MAIN_GRAPHICS_RESOURCE_FACTORY_REF_HPP
#define TERRAFORMER_UI_MAIN_GRAPHICS_RESOURCE_FACTORY_REF_HPP

#include "./texture.hpp"

namespace terraformer::ui::main
{
	class graphics_resource_factory_ref
	{
	public:
		template<class FactoryType>
		explicit graphics_resource_factory_ref(FactoryType& factory):
			m_handle{&factory},
			m_vtable_pointer{&s_vtable<FactoryType>},
			m_global_id{std::as_const(factory).get_global_id()}
		{}

		texture create(std::type_identity<texture>, image const& src)
		{ return m_vtable_pointer->create_texture_from_image(m_handle, m_global_id, src);}

		uint64_t get_global_id() const
		{ return m_global_id; }

	private:
		struct vtable
		{
			texture (*create_texture_from_image)(void*, uint64_t, image const&);
		};

		template<class FactoryType>
		static constexpr vtable s_vtable{
			.create_texture_from_image = [](void* object, uint64_t global_id, image const& img) {
				return static_cast<FactoryType*>(object)
					->create(std::type_identity<texture>{}, global_id, img);
			}
		};

		void* m_handle;
		vtable const* m_vtable_pointer;
		uint64_t m_global_id;
	};
}

#endif