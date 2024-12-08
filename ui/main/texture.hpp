#ifndef TERRAFORMER_UI_MAIN_TEXTURE_HPP
#define TERRAFORMER_UI_MAIN_TEXTURE_HPP

#include "lib/pixel_store/image.hpp"

#include <utility>

namespace terraformer::ui::main
{
	class texture
	{
	public:
		template<class RealTexture>
		explicit texture(std::in_place_type_t<RealTexture>,  Args&&... args):
			m_handle{resource_owner<vtable>::create(std::forward<Args>(args)...)},
			m_factory_id{factory.get_global_id()}
		{}

		void upload(span_2d<rgba_pixel const> pixels)
		{ m_handle->call(vtable::upload, pixels); }

		void bind(int slot)
		{ m_vtable->bind(m_handle.get(), slot); }

		bool created_by_factory(uint64_t factory) const
		{ return static_cast<bool>(m_handle) && m_factory_id == factory; }

	private:
		struct vtable
		{
			void (*upload)(void*, span_2d<rgba_pixel const>);
			void (*bind)(void, int);
		};

		resource_owner<vtable> m_handle;
		uint64_t m_factory_id;
	};
}

#endif