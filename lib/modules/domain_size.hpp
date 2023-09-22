#ifndef TERRAFORMER_DOMAIN_SIZE_HPP
#define TERRAFORMER_DOMAIN_SIZE_HPP

#include "lib/formbuilder/formfield.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/formbuilder/string_converter.hpp"

#include <cmath>

namespace terraformer
{
	struct domain_size
	{
		float width;
		float height;
		int number_of_pixels;
	};

	inline span_2d_extents image_size(domain_size const& dom_size)
	{
		auto const ratio = static_cast<double>(dom_size.width)/static_cast<double>(dom_size.height);
		auto const pixel_area = static_cast<double>(dom_size.number_of_pixels);
		return span_2d_extents{
			.width = static_cast<uint32_t>(std::sqrt(ratio*pixel_area) + 0.5),
			.height = static_cast<uint32_t>(std::sqrt(pixel_area/ratio) + 0.5)
		};
	}

	inline float pixel_size(domain_size const& dom_size)
	{
		return std::sqrt(dom_size.height*dom_size.width/static_cast<float>(dom_size.number_of_pixels));
	}

	uint32_t image_width(domain_size const& dom_size)
	{ return image_size(dom_size).width; }

	uint32_t image_height(domain_size const& dom_size)
	{ return image_size(dom_size).height; }

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, domain_size>)
	void bind(Form& form, std::reference_wrapper<T> dom_size)
	{
		form.insert(
			field{
				.name = "width",
				.display_name = "Width",
				.description = "Sets the width of the domain",
				.widget = textbox{
					.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = 0.0f,
							.max = std::numeric_limits<float>::infinity()
						}
					},
					.binding = std::ref(dom_size.get().width)
				}
			}
		);

		form.insert(
			field{
				.name = "height",
				.display_name = "Height",
				.description = "Sets the width of the domain",
				.widget = textbox{
					.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = 0.0f,
							.max = std::numeric_limits<float>::infinity()
						}
					},
					.binding = std::ref(dom_size.get().height)
				}
			}
		);

		form.insert(
			field{
				.name = "number_of_pixels",
				.display_name = "Number of pixel",
				.description = "Sets the number of pixels in the generated images",
				.widget = textbox{
					.value_converter = num_string_converter{
						.range = closed_closed_interval{
							.min = 1,
							.max = 8192*8192,
						}
					},
					.binding = std::ref(dom_size.get().number_of_pixels)
				}
			}
		);

		// TODO These two should use a compound output display {
		form.insert(
			field{
				.name = "image_width",
				.display_name = "Image width",
				.description = "The number of columns in the generated images",
				.widget = text_display{
					.source = [](domain_size const& dom_size){
						return to_string_helper(image_width(dom_size));
					},
					.binding = std::cref(dom_size)
				}
			}
		);

		form.insert(
			field{
				.name = "image_height",
				.display_name = "Image height",
				.description = "The number of canlines in the generated images",
				.widget = text_display{
					.source = [](domain_size const& dom_size){
						return to_string_helper(image_height(dom_size));
					},
					.binding = std::cref(dom_size)
				}
			}
		);
		// }

		form.insert(
			field{
				.name = "pixel_size",
				.display_name = "Pixel size",
				.description = "The physical size of a pixel",
				.widget = text_display{
					.source = [](domain_size const& dom_size){
						return to_string_helper(pixel_size(dom_size));
					},
					.binding = std::cref(dom_size)
				}
			}
		);
	}
}

#endif
