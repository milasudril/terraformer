//@	{"dependencies_extra": [{"ref": "./ridge_tree_generator.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_RIDGE_TREE_GENERATOR_HPP
#define TERRAFORMER_RIDGE_TREE_GENERATOR_HPP

#include "lib/common/spaces.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/math_utils/trigfunc.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/common/bounded_value.hpp"
#include "lib/common/utils.hpp"
#include "lib/generators/domain/domain_size.hpp"
#include <numbers>

namespace terraformer
{
	struct ridge_tree_trunk_control_point_descriptor
	{
		float x = 0.5f;
		float y = 0.5f;
		float heading = 0.25f;
		float speed = 0.25f;

		bool operator==(ridge_tree_trunk_control_point_descriptor const&) const = default;
		bool operator!=(ridge_tree_trunk_control_point_descriptor const&) const = default;
		void bind(descriptor_editor_ref editor);
	};

	struct ridge_tree_trunk_descriptor
	{
		ridge_tree_trunk_control_point_descriptor begin{
			.x = 0.125f,
			.y = 3.0f/8.0f,
			.heading = 0.25f,
			.speed = 1.0f,
		};

		ridge_tree_trunk_control_point_descriptor end{
			.x = 0.875f,
			.y = 5.0f/8.0f,
			.heading = 0.25f,
			.speed = 1.0f
		};

		bool operator==(ridge_tree_trunk_descriptor const&) const = default;
		bool operator!=(ridge_tree_trunk_descriptor const&) const = default;
	};

	struct ridge_tree_branch_growth_descriptor
	{
		float e2e_distance = 16384.0f;

		bool operator==(ridge_tree_branch_growth_descriptor const&) const = default;
		bool operator!=(ridge_tree_branch_growth_descriptor const&) const = default;
		void bind(descriptor_editor_ref editor);
	};

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

	struct ridge_tree_elevation_profile_descriptor
	{
		float ridge_elevation = 2048.0f;
		float ridge_half_thickness = 2048.0f + 512.0f;
		float ridge_rolloff_exponent = 1.0f;
		float noise_wavelength = 1024.0f*2.0f*std::numbers::pi_v<float>;
		float noise_lf_rolloff = 2.0f;
		float noise_hf_rolloff = 2.0f;
		float noise_amplitude = 512.0f;

		bool operator==(ridge_tree_elevation_profile_descriptor const&) const = default;
		bool operator!=(ridge_tree_elevation_profile_descriptor const&) const = default;
		void bind(descriptor_editor_ref editor);
	};

	inline constexpr float get_min_pixel_size(ridge_tree_elevation_profile_descriptor const& item)
	{
		return get_min_pixel_size(
			terraformer::wave_descriptor{
				.amplitude = item.ridge_elevation,
				.wavelength = item.noise_wavelength,
				.hf_rolloff = item.noise_hf_rolloff
			}
		);
	}

	inline constexpr float get_min_pixel_size(
		ridge_tree_branch_horz_displacement_descriptor const& a,
		ridge_tree_elevation_profile_descriptor const& b
	)
	{ return std::min(0.5f*get_min_pixel_size(a), get_min_pixel_size(b)); }

	struct ridge_tree_descriptor
	{
		std::array<std::byte, 16> rng_seed{};
		ridge_tree_trunk_descriptor trunk;

		static constexpr size_t num_levels = 3;
		std::array<ridge_tree_branch_growth_descriptor, num_levels - 1> branch_growth_params{
			ridge_tree_branch_growth_descriptor{.e2e_distance = 16384.0f},
			ridge_tree_branch_growth_descriptor{.e2e_distance = 3072.0f}
		};

		static constexpr auto default_trunk_horz_wavelength = 3.0f*2.0f*(1024.0f + 256.0f);
		static constexpr auto default_trunk_horz_amplitude = wavelength_to_amplitude(
			default_trunk_horz_wavelength,
			1.0f
		);
		static constexpr auto default_trunk_ridge_elevation = 2048.0f;
		static constexpr auto default_trunk_ridge_rolloff_exponent = 2.0f;
		static constexpr auto default_trunk_noise_wavelength = default_trunk_horz_wavelength*std::numbers::phi_v<float>;
		static constexpr auto default_trunk_noise_amplitude = 512.0f;

		static constexpr auto default_branch_1_horz_wavelength = 3072.0f;
		static constexpr auto default_branch_1_horz_amplitude = wavelength_to_amplitude(default_branch_1_horz_wavelength, 1.0f);
		static constexpr auto default_branch_1_ridge_elevation = 1024.0f;
		static constexpr auto default_branch_1_ridge_rolloff_exponent = 2.0f;
		static constexpr auto default_branch_1_noise_wavelength = default_branch_1_horz_wavelength*std::numbers::phi_v<float>;
		static constexpr auto default_branch_1_noise_amplitude = 256.0f;

		static constexpr auto default_branch_2_horz_wavelength = 600.0f;
		static constexpr auto default_branch_2_horz_amplitude = wavelength_to_amplitude(default_branch_2_horz_wavelength, 1.0f);
		static constexpr auto default_branch_2_ridge_elevation = 512.0f;
		static constexpr auto default_branch_2_ridge_rolloff_exponent = 2.0f;
		static constexpr auto default_branch_2_noise_wavelength = default_branch_2_horz_wavelength*std::numbers::phi_v<float>;
		static constexpr auto default_branch_2_noise_amplitude = 128.0f;

		std::array<ridge_tree_branch_horz_displacement_descriptor, num_levels> horz_displacements{
			ridge_tree_branch_horz_displacement_descriptor{
				.amplitude = default_trunk_horz_amplitude,
				.wavelength = default_trunk_horz_wavelength,
				.damping = {}
			},
			ridge_tree_branch_horz_displacement_descriptor{
				.amplitude = default_branch_1_horz_amplitude,
				.wavelength = default_branch_1_horz_wavelength,
				.damping = {}
			},
			ridge_tree_branch_horz_displacement_descriptor{
				.amplitude = default_branch_2_horz_amplitude,
				.wavelength = default_branch_2_horz_wavelength,
				.damping = {}
			}
		};

		std::array<ridge_tree_elevation_profile_descriptor, num_levels> elevation_profile{
			ridge_tree_elevation_profile_descriptor{
				.ridge_elevation = default_trunk_ridge_elevation,
				.ridge_half_thickness = default_trunk_ridge_elevation*(
					  default_trunk_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_trunk_noise_amplitude, default_trunk_noise_wavelength)
					+ default_branch_1_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_branch_1_noise_amplitude, default_branch_1_noise_wavelength)
				),
				.ridge_rolloff_exponent = default_trunk_ridge_rolloff_exponent,
				.noise_wavelength = default_trunk_noise_wavelength,
				.noise_lf_rolloff = 2.0f,
				.noise_hf_rolloff = 2.0f,
				.noise_amplitude = default_trunk_noise_amplitude
			},
			ridge_tree_elevation_profile_descriptor{
				.ridge_elevation = default_branch_1_ridge_elevation,
				.ridge_half_thickness = default_branch_1_ridge_elevation*(
					  default_branch_1_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_branch_1_noise_amplitude, default_branch_1_noise_wavelength)
					+ default_branch_2_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_branch_2_noise_amplitude, default_branch_2_noise_wavelength)
				),
				.ridge_rolloff_exponent = default_branch_1_ridge_rolloff_exponent,
				.noise_wavelength = default_branch_1_noise_wavelength,
				.noise_lf_rolloff = 2.0f,
				.noise_hf_rolloff = 2.0f,
				.noise_amplitude = default_branch_1_noise_amplitude
			},
			ridge_tree_elevation_profile_descriptor{
				.ridge_elevation = default_branch_2_ridge_elevation,
				.ridge_half_thickness =default_branch_2_ridge_elevation*(
					  default_branch_2_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_branch_2_noise_amplitude, default_branch_2_noise_wavelength)
				),
				.ridge_rolloff_exponent = default_branch_2_ridge_rolloff_exponent,
				.noise_wavelength = default_branch_2_noise_wavelength,
				.noise_lf_rolloff = 2.0f,
				.noise_hf_rolloff = 2.0f,
				.noise_amplitude = default_branch_2_noise_amplitude
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