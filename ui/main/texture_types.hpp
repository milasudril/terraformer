#ifndef TERRAFORMER_UI_MAIN_TEXTURE_TYPES_HPP
#define TERRAFORMER_UI_MAIN_TEXTURE_TYPES_HPP

#include "./texture.hpp"
#include "./staged_resource.hpp"

#include <memory>

namespace terraformer::ui::main
{
	using staged_texture = staged_resource<texture, image>;

	using immutable_shared_texture = std::shared_ptr<staged_texture const>;
	using shared_texture = std::shared_ptr<staged_texture>;
	using unique_texture = staged_texture;
}

#endif