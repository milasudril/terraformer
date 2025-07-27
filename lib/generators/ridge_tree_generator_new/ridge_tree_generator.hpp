//@	{"dependencies_extra": [{"ref": "./ridge_tree_generator.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_RIDGE_TREE_GENERATOR_HPP
#define TERRAFORMER_RIDGE_TREE_GENERATOR_HPP

#include "lib/generators/ridge_tree_generator_new/ridge_tree.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/common/bounded_value.hpp"
#include "lib/common/utils.hpp"
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

	inline constexpr float get_min_pixel_size(ridge_tree_branch_horz_displacement_descriptor const& item)
	{
		return get_min_pixel_size(
			terraformer::wave_descriptor{
				.amplitude = item.amplitude,
				.wavelength = item.wavelength,
				.hf_rolloff = 2.0f
			}
		);
	}

	struct ridge_tree_horz_layout_descriptor
	{
		float e2e_distance = 49152.0f;
		ridge_tree_branch_horz_displacement_descriptor displacement;

		bool operator==(ridge_tree_horz_layout_descriptor const&) const = default;
		bool operator!=(ridge_tree_horz_layout_descriptor const&) const = default;
		void bind(descriptor_editor_ref editor);
	};

	inline constexpr float get_min_pixel_size(ridge_tree_horz_layout_descriptor const& item)
	{ return get_min_pixel_size(item.displacement); }

	struct ridge_tree_elevation_profile_descriptor
	{
		float ridge_elevation = 2048.0f;
		float noise_amplitude = 512.0f;
		float lf_rolloff = 2.0f;
		float hf_rolloff = 2.0f;
		float horizontal_scale = 1024.0f*2.0f*std::numbers::pi_v<float>;
		float shape_exponent = 2.0f;

		bool operator==(ridge_tree_elevation_profile_descriptor const&) const = default;
		bool operator!=(ridge_tree_elevation_profile_descriptor const&) const = default;
		void bind(descriptor_editor_ref editor);
	};

	inline constexpr float get_min_pixel_size(ridge_tree_elevation_profile_descriptor const& item)
	{
		return get_min_pixel_size(
			terraformer::wave_descriptor{
				.amplitude = item.noise_amplitude,
				.wavelength = item.horizontal_scale,
				.hf_rolloff = item.hf_rolloff
			}
		);
	}

	inline constexpr float get_min_pixel_size(
		ridge_tree_horz_layout_descriptor const& a,
		ridge_tree_elevation_profile_descriptor const& b
	)
	{ return std::min(0.5f*get_min_pixel_size(a), get_min_pixel_size(b)); }

	inline constexpr float get_min_pixel_size(
		ridge_tree_elevation_profile_descriptor const& a,
		ridge_tree_horz_layout_descriptor const& b
	)
	{ return get_min_pixel_size(b, a); }

	struct ridge_tree_descriptor
	{
		std::array<std::byte, 16> rng_seed{};
		float x_0 = 0.0f;
		float y_0 = 0.0f;
		float heading = 0.25f;

		static constexpr size_t num_levels = 4;

		std::array<ridge_tree_horz_layout_descriptor, num_levels> horizontal_layout{
			ridge_tree_horz_layout_descriptor{
				.e2e_distance = 49152.0f,
				.displacement = ridge_tree_branch_horz_displacement_descriptor{
					.amplitude = 8192.0f/(2.0f*std::numbers::pi_v<float>),
					.wavelength = 8192.0f,
					.damping = {}
				}
			},
			ridge_tree_horz_layout_descriptor{
				.e2e_distance = 49152.0f/4.0f,
				.displacement = ridge_tree_branch_horz_displacement_descriptor{
					.amplitude = 2048.0f/(2.0f*std::numbers::pi_v<float>),
					.wavelength = 2048.0f,
					.damping = {}
				}
			},
			ridge_tree_horz_layout_descriptor{
				.e2e_distance = 49152.0f/16.0f,
				.displacement = ridge_tree_branch_horz_displacement_descriptor{
					.amplitude = 512.0f/(2.0f*std::numbers::pi_v<float>),
					.wavelength = 512.0f,
					.damping = {}
				}
			},
			ridge_tree_horz_layout_descriptor{
				.e2e_distance = 49152.0f/64.0f,
				.displacement = ridge_tree_branch_horz_displacement_descriptor{
					.amplitude = 128.0f/(2.0f*std::numbers::pi_v<float>),
					.wavelength = 128.0f,
					.damping = {}
				}
			}
		};

		std::array<ridge_tree_elevation_profile_descriptor, num_levels> elevation_profile{
			ridge_tree_elevation_profile_descriptor{
				.ridge_elevation = 2048.0f,
				.noise_amplitude = 512.0f,
				.lf_rolloff = 2.0f,
				.hf_rolloff = 2.0f,
				.horizontal_scale = 1024.0f*2.0f*std::numbers::pi_v<float>,
				.shape_exponent = 2.0f
			},
			ridge_tree_elevation_profile_descriptor{
				.ridge_elevation = 1024.0f,
				.noise_amplitude = 256.0f,
				.lf_rolloff = 2.0f,
				.hf_rolloff = 2.0f,
				.horizontal_scale = 512.0f*2.0f*std::numbers::pi_v<float>,
				.shape_exponent = 2.0f
			},
			ridge_tree_elevation_profile_descriptor{
				.ridge_elevation = 512.0f,
				.noise_amplitude = 128.0f,
				.lf_rolloff = 2.0f,
				.hf_rolloff = 2.0f,
				.horizontal_scale = 256.0f*2.0f*std::numbers::pi_v<float>,
				.shape_exponent = 2.0f
			},
			ridge_tree_elevation_profile_descriptor{
				.ridge_elevation = 256.0f,
				.noise_amplitude = 64.0f,
				.lf_rolloff = 2.0f,
				.hf_rolloff = 2.0f,
				.horizontal_scale = 128.0f*2.0f*std::numbers::pi_v<float>,
				.shape_exponent = 2.0f
			}
		};

		bool operator==(ridge_tree_descriptor const&) const = default;
		bool operator!=(ridge_tree_descriptor const&) const = default;

		grayscale_image generate_heightmap(domain_size_descriptor) const;
		void bind(descriptor_editor_ref editor);
	};

	float get_min_pixel_size(ridge_tree_descriptor const& params);

	grayscale_image generate(domain_size_descriptor dom_size, ridge_tree_descriptor const& params);
}

#endif