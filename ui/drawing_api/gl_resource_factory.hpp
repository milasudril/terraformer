#ifndef TERRAFORMER_UI_DRAWING_API_GL_RESOURCE_FACTORY_HPP
#define TERRAFORMER_UI_DRAWING_API_GL_RESOURCE_FACTORY_HPP

#include "./gl_texture.hpp"

#include "ui/main/texture.hpp"
#include "lib/common/global_instance_counter.hpp"

namespace terraformer::ui::drawing_api
{
	class gl_resource_factory:public global_instance_counter
	{
	public:
		static main::texture create(std::type_identity<main::texture>, uint64_t factory_id, image const& src)
		{
			return main::texture{std::in_place_type_t<gl_texture>{}, factory_id,  src.pixels()};
		}
	};
}

#endif