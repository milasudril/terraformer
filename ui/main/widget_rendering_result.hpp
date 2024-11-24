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

		set_texture_result set_widget_background(
			generic_texture_pointer_const texture,
			std::array<rgba_pixel, 4> const& tints
		)
		{ return m_vtable->set_widget_background(m_pointer, texture, tints); }

		set_texture_result set_bg_layer_mask(generic_texture_pointer_const texture)
		{ return m_vtable->set_bg_layer_mask(m_pointer, texture); }

		set_texture_result set_selection_background(
			generic_texture_pointer_const texture,
			std::array<rgba_pixel, 4> const& tints
		)
		{ return m_vtable->set_selection_background(m_pointer, texture, tints); }

		set_texture_result set_widget_foreground(
			generic_texture_pointer_const texture,
			std::array<rgba_pixel, 4> const& tints,
			displacement offset
		)
		{ return m_vtable->set_widget_foreground(m_pointer, texture, tints, offset); }

		set_texture_result set_frame(
			generic_texture_pointer_const texture,
			std::array<rgba_pixel, 4> const& tints
		)
		{ return m_vtable->set_frame(m_pointer, texture, tints); }

		set_texture_result set_input_marker(
			generic_texture_pointer_const texture,
			std::array<rgba_pixel, 4> const& tints,
			displacement offset
		)
		{ return m_vtable->set_input_marker(m_pointer, texture, tints, offset); }

	private:
		struct vtable
		{
			template<class ... ExtraArgs>
			using set_layer_callback = set_texture_result (*)(
				void*,
				generic_texture_pointer_const,
				std::array<rgba_pixel, 4> const&,
				ExtraArgs...
			);

			generic_unique_texture (*create_texture)();
			set_layer_callback<> set_widget_background;
			set_texture_result (*set_bg_layer_mask)(void*, generic_texture_pointer_const);
			set_layer_callback<> set_selection_background;
			set_layer_callback<displacement> set_widget_foreground;
			set_layer_callback<displacement> set_input_marker;
			set_layer_callback<> set_frame;
		};

		template<class T>
		static constexpr vtable s_vt{
			.create_texture = [](){
				return generic_unique_texture{std::type_identity<typename T::texture_type>{}};
			},
			.set_widget_background = [](void* obj, generic_texture_pointer_const texture, std::array<rgba_pixel, 4> const& tints) {
				return static_cast<T*>(obj)->set_widget_background(texture.get_if<typename T::texture_type>(), tints);
			},
			.set_bg_layer_mask = [](void* obj, generic_texture_pointer_const texture) {
				return static_cast<T*>(obj)->set_bg_layer_mask(texture.get_if<typename T::texture_type>());
			},
			.set_selection_background = [](void* obj, generic_texture_pointer_const texture, std::array<rgba_pixel, 4> const& tints){
				return static_cast<T*>(obj)->set_selection_background(texture.get_if<typename T::texture_type>(), tints);
			},
			.set_widget_foreground = [](
				void* obj,
				generic_texture_pointer_const texture,
				std::array<rgba_pixel, 4> const& tints,
				displacement offset
			){
				return static_cast<T*>(obj)->set_widget_foreground(
					texture.get_if<typename T::texture_type>(),
					tints,
					offset
				);
			},
			.set_input_marker =  [](
				void* obj,
				generic_texture_pointer_const texture,
				std::array<rgba_pixel, 4> const& tints,
				displacement offset
			){
				return static_cast<T*>(obj)->set_input_marker(
					texture.get_if<typename T::texture_type>(),
					tints,
					offset
				);
			},
			.set_frame = [](void* obj, generic_texture_pointer_const texture, std::array<rgba_pixel, 4> const& tints){
				return static_cast<T*>(obj)->set_frame(texture.get_if<typename T::texture_type>(), tints);
			}
		};
		vtable const* m_vtable;
		void* m_pointer;
	};
}

#endif