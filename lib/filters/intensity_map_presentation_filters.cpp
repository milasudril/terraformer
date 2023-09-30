//@	{"target":{"name": "intensity_map_presentation_filters.o"}}

#include "./intensity_map_presentation_filters.hpp"
#include "lib/common/utils.hpp"

#include <cassert>

terraformer::grayscale_image
terraformer::resize(grayscale_image const& src, image_resize_description const& resize_params)
{
	auto const container_width = static_cast<double>(resize_params.output_width);
	auto const container_height = static_cast<double>(resize_params.output_height);

	auto const w = static_cast<double>(src.width());
	auto const h = static_cast<double>(src.height());

	auto const input_ratio = w/h;

	auto const output_width = std::min(input_ratio*container_height, container_width);
	auto const output_height = std::min(container_width/input_ratio, container_height);
	auto const r = w/output_width;

	grayscale_image ret{static_cast<uint32_t>(output_width), static_cast<uint32_t>(output_height)};

	if(r >= 1.0)
	{
		for(uint32_t y = 0; y != ret.height(); ++y)
		{
			for(uint32_t x = 0; x != ret.width(); ++x)
			{
				auto const src_x = static_cast<uint32_t>(static_cast<double>(x)*r);
				auto const src_y = static_cast<uint32_t>(static_cast<double>(y)*r);
				auto const src_x_end = static_cast<uint32_t>(static_cast<double>(x + 1)*r);
				auto const src_y_end = static_cast<uint32_t>(static_cast<double>(y + 1)*r);

				auto output_value = 0.0f;
				for(uint32_t k = src_y; k != src_y_end; ++k)
				{
					for(uint32_t l = src_x; l != src_x_end; ++l)
					{ output_value += src(l, k); }
				}

				ret(x, y) = output_value/static_cast<float>(((src_x_end - src_x)*(src_y_end - src_y)));
			}
		}
	}
	else
	{
		for(uint32_t y = 0; y != ret.height(); ++y)
		{
			for(uint32_t x = 0; x != ret.width(); ++x)
			{
				auto const src_x = static_cast<uint32_t>(static_cast<double>(x)*r);
				auto const src_y = static_cast<uint32_t>(static_cast<double>(y)*r);
				ret(x, y) = src(src_x, src_y);
			}
		}
	}

	return ret;
}

terraformer::grayscale_image
terraformer::posterize(grayscale_image const& src, posterization_description const& params)
{
	terraformer::grayscale_image ret{src.width(), src.height()};
	auto const range = std::ranges::minmax_element(src.pixels());
	auto const min = *range.min;
	auto const z_range = *range.max - min;

	if(z_range == 0.0f)
	{ return ret; }

	auto const levels = static_cast<float>(params.levels);

	for(uint32_t y = 0; y != ret.height(); ++y)
	{
		for(uint32_t x = 0; x != ret.width(); ++x)
		{
			auto const val = static_cast<int>(levels*(src(x, y) - min)/z_range + 0.5f)/levels;
			ret(x, y) = val;
		}
	}
	return ret;
}

terraformer::grayscale_image terraformer::generate_level_curves(grayscale_image const& src,
	posterization_description const& params)
{
	auto img_posterized = posterize(src, params);
	terraformer::grayscale_image ret{src.width(), src.height()};
	for(uint32_t y = 1; y != ret.height() - 1; ++y)
	{
		for(uint32_t x = 1; x != ret.width() - 1; ++x)
		{
			auto ddx = img_posterized(x + 1, y) - img_posterized(x - 1, y);
			auto ddy = img_posterized(x, y + 1) - img_posterized(x, y - 1);

			ret(x, y) = ddx*ddx + ddy*ddy > 0.0f;
		}
	}
	return ret;
}

terraformer::image terraformer::apply_colormap(grayscale_image const& src,
	std::span<rgba_pixel const> colors)
{
	terraformer::image ret{src.width(), src.height()};
	auto const range = std::ranges::minmax_element(src.pixels());
	auto const min = *range.min;
	auto const z_range = *range.max - min;
	if(z_range == 0.0f)
	{ return ret; }

	for(uint32_t y = 0; y != ret.height(); ++y)
	{
		for(uint32_t x = 0; x != ret.width(); ++x)
		{
			auto const xi = static_cast<float>(std::size(colors) - 1)*(src(x, y) - min)/z_range;
			ret(x, y) = interp(colors, xi);
		}
	}

	return ret;
}