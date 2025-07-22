//@	{"target": {"name": "./ridge_tree_generator.o"}}

#include "./ridge_tree_generator.hpp"

#include "lib/common/rng.hpp"

#include <cassert>
#include <random>

terraformer::grayscale_image
terraformer::generate(domain_size_descriptor, ridge_tree_descriptor const&)
{
	terraformer::grayscale_image ret{1, 1};
	return ret;
}

void terraformer::ridge_tree_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_rng_seed_input(u8"Seed", rng_seed);
}

terraformer::grayscale_image
terraformer::ridge_tree_descriptor::generate_heightmap(domain_size_descriptor size) const
{ return generate(size, *this); }
