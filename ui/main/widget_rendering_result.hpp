#ifndef TERRAFORMER_UI_MAIN_WIDGET_RENDERING_RESULT_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_RENDERING_RESULT_HPP

#include "./generic_texture.hpp"

namespace terraformer::ui::main
{
	// TODO: Need to have a dumb pointer to a generic texture as well. This way,
	// set_background/set_foreground would be more type-safe
	class widget_rendering_result
	{
	public:
		template<class T>
		explicit widget_rendering_result(std::reference_wrapper<T> obj):
			m_vtable{s_vt<T>},
			m_pointer{&obj.get()}
		{}

		generic_unique_texture create_texture()
		{ return m_vtable->create_texture(); }

		void set_background(any_pointer_to_const texture)
		{ m_vtable->set_background(m_pointer, texture); }

		void set_foreground(any_pointer_to_const texture)
		{ m_vtable->set_foreground(m_pointer, texture); }

		void set_background_tints(std::array<rgba_pixel, 4> const& vals)
		{ m_vtable->set_background_tints(m_pointer, vals); }

		void set_foreground_tints(std::array<rgba_pixel, 4> const& vals)
		{ m_vtable->set_foreground_tints(m_pointer, vals); }

	private:
		struct vtable
		{
			generic_unique_texture (*create_texture)();
			void (*set_background)(void*, any_pointer_to_const texture);
			void (*set_foreground)(void*, any_pointer_to_const texture);
			void (*set_background_tints)(void*, std::array<rgba_pixel, 4> const&);
			void (*set_foreground_tints)(void*, std::array<rgba_pixel, 4> const&);
		};

		template<class T>
		static constexpr vtable s_vt{
			.create_texture = [](){
				return generic_unique_texture{std::type_identity<typename T::texture_type>{}};
			},
			.set_background = [](void* obj, any_pointer_to_const texture){
				static_cast<T*>(obj)->set_background(texture.get_if<typename T::texture_type>());
			},
			.set_foreground = [](void* obj, any_pointer_to_const texture){
				static_cast<T*>(obj)->set_foreground(texture.get_if<typename T::texture_type>());
			},
			.set_background_tints = [](void* obj, std::array<rgba_pixel, 4> const& vals){
				static_cast<T*>(obj)->set_background_tints(vals);
			},
			.set_foreground_tints = [](void* obj, std::array<rgba_pixel, 4> const& vals){
				static_cast<T*>(obj)->set_foreground_tints(vals);
			}
		};
		vtable const* m_vtable;
		void* m_pointer;
	};
}

#endif