//@	{
//@		"target":{
//@			"name":"form.o"
//@		},
//	NOTE: We need to change some compiler flags for Qt
//@		"compiler":{
//@			"config": {
//@				"cflags":[
//	Qt headers uses some ugly implicit conversions
//@					"-Wno-error=conversion",
//	Qt headers uses enum conversions that have been deprecated in C++20
//@					"-Wno-error=deprecated-enum-enum-conversion",
//	Qt requires that we build with fpic
//@					"-fpic"
//@				]
//@			}
//@		}
//@	}

#include "./form.hpp"
#include "lib/filters/intensity_map_presentation_filters.hpp"

namespace
{
	struct output_pixel
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};
};

void terraformer::topographic_map_renderer::upload(grayscale_image const& img)
{
	auto const img_to_present = posterize(
		terraformer::resize(img, image_resize_description{
			.output_width = static_cast<uint32_t>(m_image_view->width()),
			.output_height = static_cast<uint32_t>(m_image_view->height())
		}),
		posterization_description{
		.levels = 16
		}
	);

	basic_image<output_pixel> img_srgb{img_to_present.width(), img_to_present.height()};
	for(uint32_t y = 0; y != img_srgb.height(); ++y)
	{
		for(uint32_t x = 0; x != img_srgb.width(); ++x)
		{
			auto const output_value = static_cast<uint8_t>(255.0f*img_to_present(x, y));

			img_srgb(x, y) = output_pixel{
				.b = output_value,
				.g = output_value,
				.r = output_value,
				.a = 0xff,
			};
		}
	}

	QImage img_out{
		reinterpret_cast<uchar const*>(img_srgb.pixels().data()),
		static_cast<int>(img_srgb.width()),
		static_cast<int>(img_srgb.height()),
		QImage::Format_ARGB32
	};

	m_image_view->set_pixmap(QPixmap::fromImage(img_out));
	m_image_view->update();
}