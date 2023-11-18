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
		random_generator& rng);

	grayscale_image generate(span_2d<float const> u,
		span_2d<float const> v,
		float pxiel_size,
		float ridge_loc,
		modulated_damped_wave_description const& ns_wave_desc,
		random_generator& rng);

	grayscale_image generate(span_2d<float const> u,
		span_2d<float const> v,
		float ridge_loc,
		filtered_noise_description_2d const& bump_field_desc,
		random_generator& rng);

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
			ridge_curve{generate(hm.main_ridge, rng, dom_res.width, dom_res.pixel_size)},
			u{generate(dom_res.width, dom_res.height, pixel_size, ridge_curve, static_cast<float>(hm.main_ridge.ridge_curve_xy.initial_value))},
			v{generate(u.pixels(), pixel_size, static_cast<float>(hm.main_ridge.ridge_curve_xy.initial_value), hm.ns_distortion, rng)},
			bump_field{generate(u, v, pixel_size, hm.bump_field.wave, rng)},
			ns_wave{generate(u, v, pixel_size, static_cast<float>(hm.main_ridge.ridge_curve_xy.initial_value), hm.ns_wave, rng)}
		{ generate(*this, hm); }


		grayscale_image pixel_storage;
		float pixel_size;
		closed_closed_interval<float> output_range;

		// Cached partial results
		std::vector<location> ridge_curve;
		grayscale_image u;
		grayscale_image v;
		grayscale_image bump_field;
		grayscale_image ns_wave;

		void rng_seed_updated(initial_heightmap_description const& description, random_generator& rng)
		{ main_ridge_updated(description, rng); }

		void domain_size_updated(domain_size_description const& dom_size,
			initial_heightmap_description const& hm,
			random_generator& rng)
		{
			auto const dom_res = make_domain_resolution(dom_size);
			pixel_storage = grayscale_image{dom_res.width, dom_res.height};
			pixel_size = dom_res.pixel_size;
			main_ridge_updated(hm, rng);
		}

		void output_range_updated(initial_heightmap_description const& hm,
			random_generator&)
		{
			output_range = closed_closed_interval<float>(hm.output_range.min, hm.output_range.max);
			generate(*this, hm);
		}

		void corners_updated(initial_heightmap_description const& description, random_generator&)
		{ generate(*this, description); }

		void main_ridge_updated(initial_heightmap_description const& description, random_generator& rng)
		{
			ridge_curve = generate(description.main_ridge, rng, pixel_storage.width(), pixel_size);
			u = generate(pixel_storage.width(), pixel_storage.height(), pixel_size, ridge_curve, static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value));
			v = generate(u.pixels(), pixel_size, static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value), description.ns_distortion, rng);
			ns_wave = generate(u, v, pixel_size, static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value), description.ns_wave, rng);
			bump_field = generate(u.pixels(), v.pixels(), pixel_size, description.bump_field.wave, rng);
			generate(*this, description);
		}

		void ns_distortion_updated(initial_heightmap_description const& description, random_generator& rng)
		{
			v = generate(u.pixels(), pixel_size, static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value), description.ns_distortion, rng);
			ns_wave = generate(u, v, pixel_size, static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value), description.ns_wave, rng);
			bump_field = generate(u.pixels(), v.pixels(), pixel_size, description.bump_field.wave, rng);
			generate(*this, description);
		}

		void bump_field_updated(initial_heightmap_description const& description, random_generator& rng)
		{
			bump_field = generate(u.pixels(), v.pixels(), pixel_size, description.bump_field.wave, rng);
			generate(*this, description);
		}

		void ns_wave_updated(initial_heightmap_description const& description, random_generator& rng)
		{
			ns_wave = generate(u, v, pixel_size, static_cast<float>(description.main_ridge.ridge_curve_xy.initial_value), description.ns_wave, rng);
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