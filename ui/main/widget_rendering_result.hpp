#ifndef TERRAFORMER_UI_MAIN_WIDGET_RENDERING_RESULT_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_RENDERING_RESULT_HPP

#include "./generic_texture.hpp"

namespace terraformer::ui::main
{
	enum class set_texture_result{success, incompatible};

	class widget_rendering_result
	{
	public:
		template<class T>
		explicit widget_rendering_result(std::reference_wrapper<T> obj):
			m_vtable{&s_vt<T>},
			m_pointer{&obj.get()}
		{}

		generic_unique_texture create_texture()
		{ return m_vtable->create_texture(); }

		set_texture_result set_background(generic_texture_pointer_const texture)
		{ return m_vtable->set_background(m_pointer, texture); }

		set_texture_result set_foreground(generic_texture_pointer_const texture)
		{ return m_vtable->set_foreground(m_pointer, texture); }

		void set_background_tints(std::array<rgba_pixel, 4> const& vals)
		{ m_vtable->set_background_tints(m_pointer, vals); }

		void set_foreground_tints(std::array<rgba_pixel, 4> const& vals)
		{ m_vtable->set_foreground_tints(m_pointer, vals); }

	private:
		struct vtable
		{
			generic_unique_texture (*create_texture)();
			set_texture_result (*set_background)(void*, generic_texture_pointer_const texture);
			set_texture_result (*set_foreground)(void*, generic_texture_pointer_const texture);
			void (*set_background_tints)(void*, std::array<rgba_pixel, 4> const&);
			void (*set_foreground_tints)(void*, std::array<rgba_pixel, 4> const&);
		};

		template<class T>
		static constexpr vtable s_vt{
			.create_texture = [](){
				return generic_unique_texture{std::type_identity<typename T::texture_type>{}};
			},
			.set_background = [](void* obj, generic_texture_pointer_const texture){
				return static_cast<T*>(obj)->set_background(texture.get_if<typename T::texture_type>());
			},
			.set_foreground = [](void* obj, generic_texture_pointer_const texture){
				return static_cast<T*>(obj)->set_foreground(texture.get_if<typename T::texture_type>());
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