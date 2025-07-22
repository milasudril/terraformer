//@	{"dependencies_extra": [{"ref": "./ridge_tree_generator.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_RIDGE_TREE_GENERATOR_HPP
#define TERRAFORMER_RIDGE_TREE_GENERATOR_HPP

#include "lib/generators/domain/domain_size.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"

namespace terraformer
{
	struct ridge_tree_descriptor
	{
		std::array<std::byte, 16> rng_seed{};

		bool operator==(ridge_tree_descriptor const&) const = default;
		bool operator!=(ridge_tree_descriptor const&) const = default;

		grayscale_image generate_heightmap(domain_size_descriptor) const;
		void bind(descriptor_editor_ref editor);
	};

	grayscale_image generate(domain_size_descriptor dom_size, ridge_tree_descriptor const& params);
}

#endif