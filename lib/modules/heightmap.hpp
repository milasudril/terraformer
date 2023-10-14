//@	{"dependencies_extra":[{"ref":"./heightmap.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_HEIGHTMAP_HPP
#define TERRAFORMER_HEIGHTMAP_HPP

#include "lib/pixel_store/image.hpp"

#include "./domain_size_description.hpp"
#include "./initial_heightmap_description.hpp"

namespace terraformer
{
	struct heightmap
	{
		grayscale_image pixel_storage;
		float pixel_size;
	};

	inline heightmap make_heightmap(domain_resolution const& dom_res)
	{
		return heightmap{
			.pixel_storage = grayscale_image{dom_res.width, dom_res.height},
			.pixel_size = dom_res.pixel_size
		};
	}

	inline heightmap make_heightmap(domain_size_description const& dom_size)
	{
		return make_heightmap(make_domain_resolution(dom_size));
	}

	void generate(heightmap& output, initial_heightmap_description const& description, random_generator& rng);

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
					.value_converter = num_string_converter<float>{}
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
					.heightmap = std::ref(heightmap.get().pixel_storage)
				}
			}
		);
	}
}

#endif