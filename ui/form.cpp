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

void terraformer::colorbar::generate_image()
{
	std::span<rgba_pixel const> colors{m_colormap};

	auto const width = scale_width;
	auto const height = this->height() - 2*m_label_height;
	basic_image<output_pixel> img_srgb{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
	for(uint32_t y = 0; y != img_srgb.height(); ++y)
	{
		for(uint32_t x = 0; x != img_srgb.width(); ++x)
		{
			auto const hred = height - 1;
			auto const val = static_cast<float>(std::size(colors) - 1)
				*static_cast<float>(hred - y)/static_cast<float>(hred);
			auto const output_value = interp(colors, val);

			img_srgb(x, y) = output_pixel{
				.b = static_cast<uint8_t>(255.0f*std::pow(output_value.blue(), 1.0f/2.2f)),
				.g = static_cast<uint8_t>(255.0f*std::pow(output_value.green(), 1.0f/2.2f)),
				.r = static_cast<uint8_t>(255.0f*std::pow(output_value.red(), 1.0f/2.2f)),
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

	m_image_data = QPixmap::fromImage(img_out);
}

void terraformer::colorbar::paintEvent(QPaintEvent*)
{
	QPainter p{this};
	p.drawPixmap(0, m_label_height, m_image_data);

	auto const height = this->height() - 2*m_label_height;
	auto const N = 16;
	auto const dy = static_cast<float>(height)/static_cast<float>(N);
	auto const dv = (m_range.max - m_range.min)/static_cast<float>(N);
	auto const min = m_range.min;
	auto const em = m_em;
	auto const y_0 = static_cast<float>(m_label_height);
	auto const y_txt_ofs = 0.25f*static_cast<float>(m_digit_height);

	for(size_t k = 0; k != N + 1; ++k)
	{
		auto const x = scale_width;
		auto const y_marker = height - static_cast<int>(static_cast<float>(k)*dy - y_0);
		auto const y_txt = height - static_cast<int>(static_cast<float>(k)*dy - y_0 - y_txt_ofs);
		p.drawLine(x, y_marker, x + em, y_marker);
		auto const value = static_cast<float>(k)*dv + min;
		p.drawText(x + 2*em, y_txt, QString::fromStdString(std::to_string(value)));
	}
}

void terraformer::topographic_map_renderer::upload(grayscale_image const& img)
{
	auto const r = static_cast<double>(img.width())/static_cast<double>(img.height());
	auto const min_width = static_cast<uint32_t>(static_cast<double>(m_image_view->height())*r);
	m_image_view->setMinimumWidth(static_cast<int>(min_width));

	image_resize_description resize_op{
		.output_width = min_width,
		.output_height = static_cast<uint32_t>(m_image_view->height())
	};

	auto const fitted_image = terraformer::resize(img, resize_op);

	auto const level_curves = generate_level_curves(fitted_image,
		posterization_description{
			.levels = 16
		}
	);

	m_colorbar->set_range(level_curves.second);

	auto const mapped_intensity = apply_colormap(fitted_image, m_colormap, level_curves.second);

	basic_image<output_pixel> img_srgb{fitted_image.width(), fitted_image.height()};
	for(uint32_t y = 0; y != img_srgb.height(); ++y)
	{
		for(uint32_t x = 0; x != img_srgb.width(); ++x)
		{
			auto const output_value = mapped_intensity(x, y)*(1.0f - 0.5f*level_curves.first(x, y));

			img_srgb(x, y) = output_pixel{
				.b = static_cast<uint8_t>(255.0f*std::pow(output_value.blue(), 1.0f/2.2f)),
				.g = static_cast<uint8_t>(255.0f*std::pow(output_value.green(), 1.0f/2.2f)),
				.r = static_cast<uint8_t>(255.0f*std::pow(output_value.red(), 1.0f/2.2f)),
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