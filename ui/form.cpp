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
	auto const container_width = static_cast<double>(m_image_view->width());
	auto const container_height = static_cast<double>(m_image_view->height());

	auto const w = static_cast<double>(img.width());
	auto const h = static_cast<double>(img.height());

	auto const input_ratio = w/h;

	auto const output_width = std::min(input_ratio*container_height, container_width);
	auto const output_height = std::min(container_width/input_ratio, container_height);
	auto const r = w/output_width;

	basic_image<output_pixel> img_resampled{static_cast<uint32_t>(output_width), static_cast<uint32_t>(output_height)};
	for(uint32_t y = 0; y != img_resampled.height(); ++y)
	{
		for(uint32_t x = 0; x != img_resampled.width(); ++x)
		{
			auto const src_x = static_cast<uint32_t>(static_cast<double>(x)*r);
			auto const src_y = static_cast<uint32_t>(static_cast<double>(y)*r);

			// TODO:
			// Compute average
			// Use normalization

			auto const output_value = static_cast<uint8_t>(255.0f*img(src_x, src_y)/5120.0f);

			img_resampled(x, y) = output_pixel{
				.b = output_value,
				.g = output_value,
				.r = output_value,
				.a = 0xff,
			};
		}
	}

	QImage img_out{
		reinterpret_cast<uchar const*>(img_resampled.pixels().data()),
		static_cast<int>(img_resampled.width()),
		static_cast<int>(img_resampled.height()),
		QImage::Format_ARGB32
	};

	m_image_view->set_pixmap(QPixmap::fromImage(img_out));
	m_image_view->update();
}