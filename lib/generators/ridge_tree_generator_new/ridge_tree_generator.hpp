//@	{"dependencies_extra": [{"ref": "./ridge_tree_generator.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_RIDGE_TREE_GENERATOR_HPP
#define TERRAFORMER_RIDGE_TREE_GENERATOR_HPP

#include "lib/pixel_store/image.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/common/bounded_value.hpp"
#include "lib/generators/domain/domain_size.hpp"
#include <numbers>

namespace terraformer
{
	struct ridge_tree_branch_horz_displacement_descriptor
	{
		float amplitude = 8192.0f/(2.0f*std::numbers::pi_v<float>);
		float wavelength = 8192.0f;
		bounded_value<open_open_interval{0.0f, 1.0f}, std::sqrt(0.5f)> damping;

		bool operator==(ridge_tree_branch_horz_displacement_descriptor const&) const = default;
		bool operator!=(ridge_tree_branch_horz_displacement_descriptor const&) const = default;
		void bind(descriptor_editor_ref editor);
	};

	struct ridge_tree_branch_horz_descriptor
	{
		float e2e_distance = 49152.0f;
		ridge_tree_branch_horz_displacement_descriptor displacement;

		bool operator==(ridge_tree_branch_horz_descriptor const&) const = default;
		bool operator!=(ridge_tree_branch_horz_descriptor const&) const = default;
		void bind(descriptor_editor_ref editor);
	};

	struct ridge_tree_trunk_descriptor
	{
		float x_0 = 0.0f;
		float y_0 = 0.0f;
		float heading = 0.25f;
		float e2e_distance = 49152.0f;
		ridge_tree_branch_horz_displacement_descriptor horz_displacement;

		bool operator==(ridge_tree_trunk_descriptor const&) const = default;
		bool operator!=(ridge_tree_trunk_descriptor const&) const = default;
		void bind(descriptor_editor_ref editor);
	};

	struct ridge_tree_descriptor
	{
		std::array<std::byte, 16> rng_seed{};

		ridge_tree_trunk_descriptor trunk;

		bool operator==(ridge_tree_descriptor const&) const = default;
		bool operator!=(ridge_tree_descriptor const&) const = default;

		grayscale_image generate_heightmap(domain_size_descriptor) const;
		void bind(descriptor_editor_ref editor);
	};

	grayscale_image generate(domain_size_descriptor dom_size, ridge_tree_descriptor const& params);
}

#endif