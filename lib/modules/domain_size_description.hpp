#ifndef TERRAFORMER_DOMAIN_SIZE_DESCRIPTION_HPP
#define TERRAFORMER_DOMAIN_SIZE_DESCRIPTION_HPP

#include "lib/formbuilder/formfield.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/formbuilder/string_converter.hpp"

#include <cmath>

namespace terraformer
{
	struct domain_size_description
	{
		float width;
		float height;
		int number_of_pixels;
	};

	inline span_2d_extents image_size(domain_size_description const& dom_size)
	{
		auto const ratio = static_cast<double>(dom_size.width)/static_cast<double>(dom_size.height);
		auto const pixel_area = static_cast<double>(dom_size.number_of_pixels);
		return span_2d_extents{
			.width = static_cast<uint32_t>(std::sqrt(ratio*pixel_area) + 0.5),
			.height = static_cast<uint32_t>(std::sqrt(pixel_area/ratio) + 0.5)
		};
	}

	inline float pixel_size(domain_size_description const& dom_size)
	{
		return std::sqrt(dom_size.height*dom_size.width/static_cast<float>(dom_size.number_of_pixels));
	}

	inline uint32_t image_width(domain_size_description const& dom_size)
	{ return image_size(dom_size).width; }

	inline uint32_t image_height(domain_size_description const& dom_size)
	{ return image_size(dom_size).height; }

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, domain_size_description>)
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
	}

	struct domain_resolution
	{
		uint32_t width;
		uint32_t height;
		float pixel_size;
	};

	inline domain_resolution make_domain_resolution(domain_size_description const& dom_size)
	{
		auto const img_size = image_size(dom_size);
		return domain_resolution{
			.width = img_size.width,
			.height = img_size.height,
			.pixel_size = pixel_size(dom_size)
		};
	}

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, domain_resolution>)
	void bind(Form& form, std::reference_wrapper<T> resolution)
	{
		form.insert(
			field{
				.name = "width",
				.display_name = "Width",
				.description = "Sets the width of the domain",
				.widget = textbox{
					.value_converter = num_string_converter{
						.range = closed_closed_interval{
							.min = 1u,
							.max = std::numeric_limits<uint32_t>::max()
						}
					},
					.binding = std::ref(resolution.get().width)
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
						.range = closed_closed_interval{
							.min = 1u,
							.max = std::numeric_limits<uint32_t>::max()
						}
					},
					.binding = std::ref(resolution.get().height)
				}
			}
		);

		form.insert(
			field{
				.name = "pixel_size",
				.display_name = "Pixel size",
				.description = "Sets the size of pixels",
				.widget = textbox{
					.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = 0.0f,
							.max = std::numeric_limits<float>::infinity(),
						}
					},
					.binding = std::ref(resolution.get().pixel_size)
				}
			}
		);
	}
}

#endif