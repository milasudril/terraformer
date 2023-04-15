//@	{
//@	"target":{"name":"image_io.o", "type":"object", "pkgconfig_libs":["OpenEXR"]},
//@	"dependencies":[{"ref":"OpenEXR", "origin":"pkg-config"}]
//@	}

#include "./image_io.hpp"
#include "./channel_mask.hpp"

#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfFrameBuffer.h>

namespace
{
	terraformer::channel_mask get_channel_mask(Imf::ChannelList const& channels)
	{
		terraformer::channel_mask ret;
		for(auto i = std::begin(channels); i != std::end(channels); ++i)
		{
			ret.set(i.name());
		}
		return ret;
	}
}

void use(terraformer::span_2d<float const>){}

terraformer::image terraformer::load(
	empty<image>,
	void* arg,
	image_io_detail::input_file_factory make_input_file)
{
	auto src = make_input_file(arg);

	auto box = src.header().dataWindow();

	auto w = box.max.x - box.min.x + 1;
	auto h = box.max.y - box.min.y + 1;

	if(w > 65535 || h > 65535)
	{ throw std::runtime_error{"Tried to load a too large image"}; }

	auto const channel_mask = get_channel_mask(src.header().channels());
	if(!represents_color_image(channel_mask))
	{ throw std::runtime_error{"Expected a color image"}; }

	image ret{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
	Imf::FrameBuffer fb;
	fb.insert("R",
		Imf::Slice{Imf::FLOAT,
			(char*)(ret.pixels().data()) + 0 * sizeof(float),
			sizeof(rgba_pixel),
			sizeof(rgba_pixel) * w});
	fb.insert("G",
		Imf::Slice{Imf::FLOAT,
			(char*)(ret.pixels().data()) + 1 * sizeof(float),
			sizeof(rgba_pixel),
			sizeof(rgba_pixel) * w});
	fb.insert("B",
		Imf::Slice{Imf::FLOAT,
			(char*)(ret.pixels().data()) + 2 * sizeof(float),
			sizeof(rgba_pixel),
			sizeof(rgba_pixel) * w});
	fb.insert("A",
		Imf::Slice{Imf::FLOAT,
		(char*)(ret.pixels().data()) + 3 * sizeof(float),
		sizeof(rgba_pixel),
		sizeof(rgba_pixel) * w});

	src.setFrameBuffer(fb);
	src.readPixels(box.min.y, box.max.y);

	if(!channel_mask.has_alpha())
	{
		std::ranges::transform(ret.pixels(), std::begin(ret.pixels()), [](auto val) {
			val.alpha(1.0f);
			return val;
		});
	}

	return ret;
}

void terraformer::store(span_2d<rgba_pixel const> pixels,
	void* arg,
	image_io_detail::output_file_factory make_output_file)
{
	Imf::Header header{static_cast<int>(pixels.width()), static_cast<int>(pixels.height())};
	header.channels().insert("R", Imf::Channel{Imf::FLOAT});
	header.channels().insert("G", Imf::Channel{Imf::FLOAT});
	header.channels().insert("B", Imf::Channel{Imf::FLOAT});
	header.channels().insert("A", Imf::Channel{Imf::FLOAT});

	Imf::FrameBuffer fb;
	fb.insert("R",
		Imf::Slice{Imf::FLOAT,
		(char*)(pixels.data()) + 0 * sizeof(float),
		sizeof(rgba_pixel),
		sizeof(rgba_pixel) * pixels.width()});
	fb.insert("G",
		Imf::Slice{Imf::FLOAT,
		(char*)(pixels.data()) + 1 * sizeof(float),
		sizeof(rgba_pixel),
		sizeof(rgba_pixel) * pixels.width()});
	fb.insert("B",
		Imf::Slice{Imf::FLOAT,
		(char*)(pixels.data()) + 2 * sizeof(float),
		sizeof(rgba_pixel),
		sizeof(rgba_pixel) * pixels.width()});
	fb.insert("A",
		Imf::Slice{Imf::FLOAT,
		(char*)(pixels.data()) + 3 * sizeof(float),
		sizeof(rgba_pixel),
		sizeof(rgba_pixel) * pixels.width()});

	auto dest = make_output_file(arg, header);
	dest.setFrameBuffer(fb);
	dest.writePixels(pixels.height());
}

terraformer::grayscale_image terraformer::load(empty<grayscale_image>,
	void* arg,
	image_io_detail::input_file_factory make_input_file)
{
	auto src = make_input_file(arg);

	auto box = src.header().dataWindow();

	auto w = box.max.x - box.min.x + 1;
	auto h = box.max.y - box.min.y + 1;

	if(w > 65535 || h > 65535)
	{ throw std::runtime_error{std::string{"Tried to load a too large image"}}; }

	auto const channel_mask = get_channel_mask(src.header().channels());
	if(!represents_grayscale_image(channel_mask))
	{ throw std::runtime_error{"Expected a grayscale image"}; }

	grayscale_image ret{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
	Imf::FrameBuffer fb;
	fb.insert("Y",
		Imf::Slice{Imf::FLOAT,
			(char*)(ret.pixels().data()) + 0 * sizeof(float),
			sizeof(float),
			sizeof(float) * w});

	src.setFrameBuffer(fb);
	src.readPixels(box.min.y, box.max.y);

	return ret;
}

void terraformer::store(span_2d<float const> pixels,
	void* arg,
	image_io_detail::output_file_factory make_output_file)
{
	Imf::Header header{static_cast<int>(pixels.width()), static_cast<int>(pixels.height())};
	header.channels().insert("Y", Imf::Channel{Imf::FLOAT});

	Imf::FrameBuffer fb;
	fb.insert("Y",
		Imf::Slice{Imf::FLOAT,
		(char*)(pixels.data()) + 0 * sizeof(float),
		sizeof(float),
		sizeof(float) * pixels.width()});

	auto dest = make_output_file(arg, header);
	dest.setFrameBuffer(fb);
	dest.writePixels(pixels.height());
}