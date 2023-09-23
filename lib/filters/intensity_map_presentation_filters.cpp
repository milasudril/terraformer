//@	{"target":{"name": "intensity_map_presentation_filters.o"}}

#include "./intensity_map_presentation_filters.hpp"

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
	for(uint32_t y = 0; y != ret.height(); ++y)
	{
		for(uint32_t x = 0; x != ret.width(); ++x)
		{
			auto const src_x = static_cast<uint32_t>(static_cast<double>(x)*r);
			auto const src_y = static_cast<uint32_t>(static_cast<double>(y)*r);
			auto const src_x_end = static_cast<uint32_t>(static_cast<double>(x + 1)*r);
			auto const src_y_end = static_cast<uint32_t>(static_cast<double>(x + 1)*r);

			auto output_value = 0.0f;
			for(uint32_t k = src_y; k != src_y_end; ++k)
			{
				for(uint32_t l = src_x; l != src_x_end; ++l)
				{ output_value += src(l, k); }
			}

			ret(x, y) = output_value/static_cast<float>(((src_x_end - src_x)*(src_y_end - src_y)));
		}
	}

	return ret;
}