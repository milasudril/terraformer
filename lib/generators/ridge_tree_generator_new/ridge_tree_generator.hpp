//@	{"dependencies_extra": [{"ref": "./ridge_tree_generator.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_RIDGE_TREE_GENERATOR_HPP
#define TERRAFORMER_RIDGE_TREE_GENERATOR_HPP

#include "lib/common/spaces.hpp"
#include "lib/generators/heightmap/heightmap_generator_context.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree_branch_seed_sequence.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/math_utils/trigfunc.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/common/bounded_value.hpp"
#include "lib/common/utils.hpp"
#include "lib/generators/domain/domain_size.hpp"
#include <geosimd/angle.hpp>
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

	struct ridge_tree_trunk_curve_descriptor
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

		bool operator==(ridge_tree_trunk_curve_descriptor const&) const = default;
		bool operator!=(ridge_tree_trunk_curve_descriptor const&) const = default;
	};

	struct ridge_tree_trunk_descriptor
	{
		ridge_tree_trunk_curve_descriptor curve;
		ridge_tree_brach_seed_sequence_boundary_point_descriptor starting_point_branches{
			.branch_count = 2,
			.spread_angle = geosimd::turn_angle{geosimd::turns{0.5f}}
		};
		float ridge_height = 2048.0f;

		bool operator==(ridge_tree_trunk_descriptor const&) const = default;
		bool operator!=(ridge_tree_trunk_descriptor const&) const = default;

		void bind(descriptor_editor_ref editor);
	};

	struct ridge_tree_branch_growth_descriptor
	{
		float e2e_distance = 16384.0f;
		float end_height = 0.5f;

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

	inline auto make_cubic_spline_control_point(
		domain_size_descriptor dom_size,
		ridge_tree_trunk_control_point_descriptor const& params
	)
	{
		auto const theta = 2.0f*std::numbers::pi_v<float>*params.heading;
		auto const r = displacement{params.x, params.y, 0.0f}
			.apply(scaling{dom_size.width, dom_size.height, 1.0f});
		auto const dom_norm = std::sqrt(dom_size.width*dom_size.height);
		return cubic_spline_control_point<location, displacement>{
			.y = location{} + r,
			.ddx = dom_norm*params.speed*displacement{std::sin(theta), -std::cos(theta), 0.0f}
		};
	}

	inline ridge_tree_trunk generate_trunk(
		domain_size_descriptor dom_size,
		ridge_tree_trunk_curve_descriptor const& base_params,
		ridge_tree_branch_horz_displacement_descriptor const& horz_displacement,
		random_generator& rng
	)
	{
		return generate_trunk(
			ridge_tree_trunk_curve{
				.begin = make_cubic_spline_control_point(dom_size, base_params.begin),
				.end = make_cubic_spline_control_point(dom_size, base_params.end)
			},
			ridge_tree_branch_displacement_description{
				.amplitude = horz_displacement.amplitude,
				.wavelength = horz_displacement.wavelength,
				.damping = horz_displacement.damping
			},
			rng
		);
	}

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

	struct ridge_tree_ridge_thickness_modulation
	{
		float begin_val;
		float end_val;
		bool rel_height;
	};

	struct ridge_tree_ridge_height_profile
	{
		float begin_height;
		bool begin_height_is_relative;
		float end_height;
		float relative_half_thickness;
		float transverse_rolloff_exponent;
		float longitudal_rolloff_exponent;
	} ;

	void fill_curve(
		span_2d<float> pixels,
		span_2d<float const> pixels_in,
		ridge_tree_trunk const& trunk,
		ridge_tree_ridge_height_profile const& elev_profile,
		float pixel_size
	);

	struct ridge_tree_elevation_modulation
	{
		float end_elevation;
		float rolloff_exponent;
	};

	inline constexpr float get_min_pixel_size(ridge_tree_elevation_profile_descriptor const& item)
	{
		return get_min_pixel_size(
			terraformer::wave_descriptor{
				.amplitude = 1024.0f,
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

		std::array<ridge_tree_brach_seed_sequence_boundary_point_descriptor, num_levels - 1> endpoint_branches{
			ridge_tree_brach_seed_sequence_boundary_point_descriptor{
				.branch_count = 2,
				.spread_angle = geosimd::turns{0.5f}
			},
			ridge_tree_brach_seed_sequence_boundary_point_descriptor{
				.branch_count = 2,
				.spread_angle = geosimd::turns{0.5f}
			}
		};

		std::array<ridge_tree_branch_growth_descriptor, num_levels - 1> branch_growth_params{
			ridge_tree_branch_growth_descriptor{.e2e_distance = 12884.0f},
			ridge_tree_branch_growth_descriptor{.e2e_distance = 3072.0f}
		};

		static constexpr auto trunk_slope = 1.25f;
		static constexpr auto default_trunk_horz_wavelength = 3.0f*2.0f*(1024.0f + 256.0f);
		static constexpr auto default_trunk_horz_amplitude = wavelength_to_amplitude(
			default_trunk_horz_wavelength,
			1.0f
		);
		static constexpr auto default_trunk_ridge_elevation = 2048.0f;
		static constexpr auto default_trunk_ridge_rolloff_exponent = 1.25f;
		static constexpr auto default_trunk_noise_wavelength = default_trunk_horz_wavelength*std::numbers::phi_v<float>;
		static constexpr auto default_trunk_noise_amplitude = 512.0f;

		static constexpr auto branch_1_slope = 1.25f;
		static constexpr auto default_branch_1_horz_wavelength = 3072.0f;
		static constexpr auto default_branch_1_horz_amplitude = wavelength_to_amplitude(default_branch_1_horz_wavelength, 1.0f);
		static constexpr auto default_branch_1_ridge_elevation = 1024.0f;
		static constexpr auto default_branch_1_ridge_rolloff_exponent = 1.25f;
		static constexpr auto default_branch_1_noise_wavelength = default_branch_1_horz_wavelength*std::numbers::phi_v<float>;
		static constexpr auto default_branch_1_noise_amplitude = 256.0f;

		static constexpr auto default_branch_2_horz_wavelength = 600.0f;
		static constexpr auto default_branch_2_horz_amplitude = wavelength_to_amplitude(default_branch_2_horz_wavelength, 1.0f);
		static constexpr auto default_branch_2_ridge_elevation = 512.0f;
		static constexpr auto default_branch_2_ridge_rolloff_exponent = 1.25f;
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
				.ridge_half_thickness = default_trunk_ridge_elevation*(
					  default_trunk_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_trunk_noise_amplitude, default_trunk_noise_wavelength)
					+ default_branch_1_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_branch_1_noise_amplitude, default_branch_1_noise_wavelength)
				)/trunk_slope,
				.ridge_rolloff_exponent = default_trunk_ridge_rolloff_exponent,
				.noise_wavelength = default_trunk_noise_wavelength,
				.noise_lf_rolloff = 1.0f,
				.noise_hf_rolloff = 2.0f,
				.noise_amplitude = default_trunk_noise_amplitude
			},
			ridge_tree_elevation_profile_descriptor{
				.ridge_half_thickness = default_branch_1_ridge_elevation*(
					  default_branch_1_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_branch_1_noise_amplitude, default_branch_1_noise_wavelength)
					+ default_branch_2_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_branch_2_noise_amplitude, default_branch_2_noise_wavelength)
				)/branch_1_slope,
				.ridge_rolloff_exponent = default_branch_1_ridge_rolloff_exponent,
				.noise_wavelength = default_branch_1_noise_wavelength,
				.noise_lf_rolloff = 1.0f,
				.noise_hf_rolloff = 2.0f,
				.noise_amplitude = default_branch_1_noise_amplitude
			},
			ridge_tree_elevation_profile_descriptor{
				.ridge_half_thickness = default_branch_2_ridge_elevation*(
					  default_branch_2_ridge_rolloff_exponent
					+ slope_from_amplitude_and_wavelength(default_branch_2_noise_amplitude, default_branch_2_noise_wavelength)
				),
				.ridge_rolloff_exponent = default_branch_2_ridge_rolloff_exponent,
				.noise_wavelength = default_branch_2_noise_wavelength,
				.noise_lf_rolloff = 1.0f,
				.noise_hf_rolloff = 2.0f,
				.noise_amplitude = default_branch_2_noise_amplitude
			}
		};

		bool operator==(ridge_tree_descriptor const&) const = default;
		bool operator!=(ridge_tree_descriptor const&) const = default;

		grayscale_image generate_heightmap(heightmap_generator_context const&) const;
		void bind(descriptor_editor_ref editor);
	};

	float get_min_pixel_size(ridge_tree_descriptor const& params);

	grayscale_image generate(heightmap_generator_context const& ctxt, ridge_tree_descriptor const& params);
}

#endif