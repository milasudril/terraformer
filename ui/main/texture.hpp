#ifndef TERRAFORMER_UI_MAIN_TEXTURE_HPP
#define TERRAFORMER_UI_MAIN_TEXTURE_HPP

#include "lib/pixel_store/image.hpp"
#include "lib/common/unique_resource.hpp"

namespace terraformer::ui::main
{
	struct texture_vtable
	{
		template<class RealTexture>
		explicit constexpr texture_vtable(std::type_identity<RealTexture>):
			upload{
				[](void* handle, span_2d<rgba_pixel const> pixels){
					static_cast<RealTexture*>(handle)->upload(pixels);
				}
			},
			bind{
				[](void* handle, int shader_port){
					static_cast<RealTexture*>(handle)->bind(shader_port);
				}
			}
		{}

		void (*upload)(void*, span_2d<rgba_pixel const>);
		void (*bind)(void*, int);
	};

	class texture_ref
	{
	public:
		texture_ref() = default;

		explicit texture_ref(resource_reference<texture_vtable> reference):
			m_reference{reference}
		{}

		void upload(span_2d<rgba_pixel const> pixels) const
		{ m_reference.get_vtable().upload(m_reference.get_pointer(), pixels); }

		void bind(int shader_port) const
		{ m_reference.get_vtable().bind(m_reference.get_pointer(), shader_port); }

		operator bool() const
		{ return static_cast<bool>(m_reference); }

	private:
		resource_reference<texture_vtable> m_reference;
	};

	class texture
	{
	public:
		texture() = default;

		template<class RealTexture, class... Args>
		explicit texture(std::in_place_type_t<RealTexture>, uint64_t factory_id, Args&&... args):
			m_handle{std::in_place_type_t<RealTexture>{}, std::forward<Args>(args)...},
			m_factory_id{factory_id}
		{}

		void upload(span_2d<rgba_pixel const> pixels)
		{ m_handle.get().get_vtable().upload(m_handle.get().get_pointer(), pixels); }

		void bind(int shader_port)
		{ m_handle.get().get_vtable().bind(m_handle.get().get_pointer(), shader_port); }

		bool created_by_factory(uint64_t factory) const
		{ return static_cast<bool>(m_handle) && m_factory_id == factory; }

		texture_ref get()
		{ return texture_ref{m_handle.get()}; }

	private:
		unique_resource<texture_vtable> m_handle;
		uint64_t m_factory_id{};
	};
}

#endif