//@	{"dependencies_extra":[{"ref":"./convhull.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTERS_CONVHULL_HPP
#define TERRAFORMER_FILTERS_CONVHULL_HPP

#include "lib/common/span_2d.hpp"
#include "lib/pixel_store/image.hpp"

#include <span>
#include <vector>

namespace terraformer
{
	void convhull(std::span<float> values);

	void convhull_per_scanline(span_2d<float> buffer);

	void convhull_per_column(span_2d<float> buffer);

	std::vector<float> convhull2(std::span<float const> values);

	basic_image<float> convhull2(span_2d<float const> input);



	inline void convhull(span_2d<float> buffer)
	{
		convhull_per_scanline(buffer);
		convhull_per_column(buffer);
	}
}

#endif
