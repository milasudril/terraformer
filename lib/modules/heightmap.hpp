//@	{"dependencies_extra":[{"ref":"./heightmap.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_HEIGHTMAP_HPP
#define TERRAFORMER_HEIGHTMAP_HPP

#include "lib/pixel_store/image.hpp"

#include "./domain_size_description.hpp"
#include "./initial_heightmap_description.hpp"
#include "./calculator.hpp"

namespace terraformer
{
	struct distance_field
	{
		grayscale_image u;
		grayscale_image v;
	};

	grayscale_image generate(uint32_t width,
		uint32_t height,
		float pixel_size,
		std::span<location const> ridge_curve,
		float ridge_loc);

	grayscale_image generate(span_2d<float const> u,
		float pixel_size,
		float ridge_loc,
		damped_wave_description const& ns_distortion,
		filtered_noise_1d_generator const& wave);

	grayscale_image generate(
		filtered_noise_2d_generator const& wave,
		span_2d<float const> u,
		span_2d<float const> v,
		float ridge_loc,
		bump_field_description const& bump_field_desc);

	struct heightmap;

	void generate(heightmap& output, initial_heightmap_description const& description);

	struct heightmap
	{
		explicit heightmap(domain_resolution const& dom_res,
			initial_heightmap_description const& hm,
			random_generator& rng):
			pixel_storage{dom_res.width, dom_res.height},
			pixel_size{dom_res.pixel_size},
			output_range{hm.output_range.min, hm.output_range.max},
			ridge_curve_rng{generate_rng_seed(rng)},
			ridge_curve_src{ridge_curve_rng, dom_res.width, pixel_size, hm.main_ridge},
			ridge_curve{generate(ridge_curve_src, hm.main_ridge.ridge_curve_xy, hm.main_ridge.ridge_curve_xz, pixel_size)},
			u{generate(dom_res.width, dom_res.height, pixel_size, ridge_curve, static_cast<float>(hm.main_ridge.ridge_curve_xy.initial_value))},
			v_rng{generate_rng_seed(rng)},
			v_wave{random_generator{v_rng}, dom_res.height, pixel_size, hm.ns_distortion.wave},
			v{generate(u.pixels(), pixel_size, static_cast<float>(hm.main_ridge.ridge_curve_xy.initial_value), hm.ns_distortion, v_wave)},
			bump_field_rng{generate_rng_seed(rng)},
			bump_field_wave{random_generator{bump_field_rng}, span_2d_extents{dom_res.width, dom_res.height}, pixel_size, hm.bump_field.wave},
			bump_field{generate(bump_field_wave, u.pixels(), v.pixels(), static_cast<float>(hm.main_ridge.ridge_curve_xy.initial_value), hm.bump_field)}
		{ generate(*this, hm); }


		grayscale_image pixel_storage;
		float pixel_size;
		closed_closed_interval<float> output_range;

		// Cached partial results
		random_generator ridge_curve_rng;
		ridge_curve_generator ridge_curve_src;
		std::vector<location> ridge_curve;

		grayscale_image u;

		random_generator v_rng;
		filtered_noise_1d_generator v_wave;
		grayscale_image v;

		random_generator bump_field_rng;
		filtered_noise_2d_generator bump_field_wave;
		grayscale_image bump_field;

		void rng_seed_updated(initial_heightmap_description const& description, random_generator& rng)
		{
			ridge_curve_rng = generate_rng_seed(rng);
			v_rng = generate_rng_seed(rng);
			bump_field_rng = generate_rng_seed(rng);
			main_ridge_updated(description);
		}

		void domain_size_updated(domain_size_description const& dom_size,
			initial_heightmap_description const& hm)
		{
			auto const dom_res = make_domain_resolution(dom_size);
			pixel_storage = grayscale_image{dom_res.width, dom_res.height};
			pixel_size = dom_res.pixel_size;
			main_ridge_updated(hm);
		}

		void output_range_updated(initial_heightmap_description const& hm)
		{
			output_range = closed_closed_interval<float>(hm.output_range.min, hm.output_range.max);
			generate(*this, hm);
		}

		void corners_updated(initial_heightmap_description const& description)
		{ generate(*this, description); }

		void main_ridge_updated(initial_heightmap_description const& description)
		{
			auto const w = pixel_storage.width();
			auto const h = pixel_storage.height();
			ridge_curve_src = ridge_curve_generator{ridge_curve_rng, w, pixel_size, description.main_ridge};
			ridge_curve = generate(ridge_curve_src, description.main_ridge.ridge_curve_xy, description.main_ridge.ridge_curve_xz, pixel_size);
			u = generate(w, h, pixel_size, ridge_curve, static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value));
			ns_distortion_updated(description);
		}

		void ns_distortion_updated(initial_heightmap_description const& description)
		{
			auto const h = pixel_storage.height();
			v_wave = filtered_noise_1d_generator{random_generator{v_rng}, h, pixel_size, description.ns_distortion.wave};
			v = generate(u.pixels(), pixel_size, static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value), description.ns_distortion, v_wave);
			bump_field_updated(description);
		}

		void bump_field_updated(initial_heightmap_description const& description)
		{
			auto const w = pixel_storage.width();
			auto const h = pixel_storage.height();
			bump_field_wave = filtered_noise_2d_generator{
				random_generator{bump_field_rng},
				span_2d_extents{w, h},
				pixel_size,
				description.bump_field.wave
			};
			bump_field = generate(bump_field_wave, u.pixels(), v.pixels(), static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value), description.bump_field);
			generate(*this, description);
		}
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, heightmap>)
	void bind(Form& form, std::reference_wrapper<T> heightmap)
	{
		form.insert(
			field{
				.name = "width",
				.display_name = "Width",
				.description = "Sets the width of the domain",
				.widget = textbox{
					.binding = std::ref(heightmap.get().pixel_storage.width_ref()),
					.value_converter = num_string_converter<int>{}
				}
			}
		);

		form.insert(
			field{
				.name = "height",
				.display_name = "Height",
				.description = "Sets the width of the domain",
				.widget = textbox{
					.binding = std::ref(heightmap.get().pixel_storage.height_ref()),
					.value_converter = num_string_converter<int>{}
				}
			}
		);

		form.insert(
			field{
				.name = "pixel_size",
				.display_name = "Pixel size",
				.description = "Sets the size of pixels",
				.widget = textbox{
					.binding = std::ref(heightmap.get().pixel_size),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "initial_heightmap",
				.display_name = "Initial heightmap",
				.description = "Shows the initial heightmap",
				.widget = topographic_map_view{
					.pixel_size = std::ref(heightmap.get().pixel_size),
					.heightmap = std::ref(heightmap.get().pixel_storage),
					.valid_range = std::ref(heightmap.get().output_range)
				}
			}
		);
	}
}

#endif