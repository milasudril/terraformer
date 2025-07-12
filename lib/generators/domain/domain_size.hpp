//@	{"dependencies_extra":[{"ref": "./domain_size.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_DOMAIN_SIZE_HPP
#define TERRAFORMER_DOMAIN_SIZE_HPP

#include "lib/descriptor_io/descriptor_editor_ref.hpp"

namespace terraformer
{
	struct domain_size_descriptor
	{
		float width = 4096.0f;
		float height = 4096.0f;

		void bind(descriptor_editor_ref editor);

		constexpr bool operator==(domain_size_descriptor const&) const = default;
		constexpr bool operator!=(domain_size_descriptor const&) const = default;
	};
}

#endif