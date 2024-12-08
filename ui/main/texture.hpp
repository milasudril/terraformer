#ifndef TERRAFORMER_UI_MAIN_TEXTURE_HPP
#define TERRAFORMER_UI_MAIN_TEXTURE_HPP

#include "lib/pixel_store/image.hpp"

#include <utility>

namespace terraformer::ui::main
{
	template<class PointerType>
	class texture
	{
	public:
		template<class RealTexture, class ... Args>
		explicit texture(std::in_place_type_t<RealTexture>, uint64_t factroy_id, Args&&... args);

		void upload(span_2d<rgba_pixel const>);

		void bind(int slot);

		bool created_by_factory(uint64_t factory) const
		{ return static_cast<bool>(m_handle) && m_factory_id == factory; }

	private:
		PointerType m_handle;
		uint64_t m_factory_id;
	};
}

#endif