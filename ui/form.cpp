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
//	QtCharts does not use override everywhere where it is applicable
//@					"-Wno-error=suggest-override",
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
	auto const height = this->height() - m_label_height;
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
	p.drawPixmap(0, m_label_height/2, m_image_data);

	auto const height = this->height() - m_label_height;
	auto const N = 16;
	auto const min = m_range.min;
	auto const max = m_range.max;
	auto const dv = (max - min)/static_cast<float>(N);
	auto const em = m_em;
	auto const y_0 = 0.5f*static_cast<float>(m_label_height);
	auto const y_txt_ofs = 0.25f*static_cast<float>(m_digit_height);

	for(size_t k = 0; k != N + 1; ++k)
	{
		auto const value = std::round(static_cast<float>(k)*dv + min);
		auto const y = std::lerp(static_cast<float>(height), 0.0f, (value - min)/(max - min));

		auto const x = scale_width;
		auto const y_marker = y + y_0;
		auto const y_txt = y_marker + y_txt_ofs;
		p.drawLine(x, static_cast<int>(y_marker), x + em, static_cast<int>(y_marker));

		p.drawText(x + 2*em, static_cast<int>(y_txt), QString::fromStdString(std::to_string(static_cast<int>(value))));
	}
}

terraformer::topographic_map_view_map_view::topographic_map_view_map_view(QWidget* parent):
	QWidget{parent},
	m_root{std::make_unique<QHBoxLayout>(this)},
	m_image_view{std::make_unique<image_view>(this)},
	m_colorbar{std::make_unique<colorbar>(this)}
{
	m_root->setContentsMargins(form_indent, 0, 0, 0);
	m_image_view->setSizePolicy(QSizePolicy{
		QSizePolicy::Policy::Fixed,
		QSizePolicy::Policy::Expanding
	});

	m_image_view->set_mouse_move_callback([this](QMouseEvent const& event) {
		if(m_heightmap != nullptr)
		{
			auto const heigtmap_displacement = m_render_scale*displacement{
				static_cast<float>(event.x()),
				static_cast<float>(event.y()),
				0.0f
			};

			if(static_cast<uint32_t>(heigtmap_displacement[0]) >= m_heightmap->width()
				|| static_cast<uint32_t>(heigtmap_displacement[1]) >= m_heightmap->height())
			{ return; }


			auto const world_displacement = m_pixel_size*heigtmap_displacement;
			auto const z_src = location{0.0f, 0.0f, 0.0f} + heigtmap_displacement;
			displacement const elevation{
				0.0f,
				0.0f,
				(*m_heightmap)(static_cast<uint32_t>(z_src[0]), static_cast<uint32_t>(z_src[1]))
			};
			auto const world_loc = location{0.0f, 0.0f, 0.0f} + world_displacement + elevation;

			auto tooltip_string = QString::fromStdString(to_string(world_loc));
			QToolTip::showText(event.globalPos(), tooltip_string);
			m_image_view->setToolTip(tooltip_string);
		}
		else
		{ m_image_view->setToolTip("No elevation data to show"); }
	});

	m_colorbar->setSizePolicy(QSizePolicy{
		QSizePolicy::Policy::Expanding,
		QSizePolicy::Policy::Expanding
	});
	m_colorbar->setToolTip("Shows mapping between color and elevation");

	m_root->addWidget(m_image_view.get());
	m_root->addWidget(m_colorbar.get());
	m_root->addSpacing(0);
	set_colormap(earth_colormap);
}


void terraformer::topographic_map_view_map_view::upload(
	std::reference_wrapper<grayscale_image const> img,
	float pixel_size)
{
	m_heightmap = &img.get();
	m_pixel_size = pixel_size;
	auto const r = static_cast<double>(img.get().width())/static_cast<double>(img.get().height());

	// TODO: Must limit min_width in case user specifies an extreme aspect ratio
	auto const min_width = static_cast<uint32_t>(static_cast<double>(m_image_view->height())*r);
	m_image_view->setMinimumWidth(static_cast<int>(min_width));

	image_resize_description resize_op{
		.output_width = min_width,
		.output_height = static_cast<uint32_t>(m_image_view->height())
	};

	auto const fitted_image = terraformer::resize(img, resize_op);
	m_render_scale = static_cast<float>(img.get().width())/static_cast<float>(resize_op.output_width);

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

void terraformer::topographic_map_xsection_diagram::upload(grayscale_image const& img, float pixel_size)
{
	auto const domain_width = pixel_size*static_cast<float>(img.width());
	auto const domain_height = pixel_size*static_cast<float>(img.height());
	m_colorbar->set_range(std::ranges::minmax_result{
		.min = 0.0f,
		.max = (m_axis_dir == axis_direction::north_to_south)? domain_height : domain_width
	});
}