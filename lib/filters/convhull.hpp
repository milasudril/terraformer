//@	{"dependencies_extra":[{"ref":"./convhull.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTERS_CONVHULL_HPP
#define TERRAFORMER_FILTERS_CONVHULL_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/double_buffer.hpp"
#include "lib/pixel_store/image.hpp"

#include <span>
#include <vector>

namespace terraformer
{
	[[nodiscard]] std::vector<float> convhull(std::span<float const> values);

	void convhull(span_2d<float const> src_buffer, span_2d<float> dest_buffer);

	[[nodiscard]] inline basic_image<float> convhull(span_2d<float const> values)
	{
		basic_image<float> ret{values.width(), values.height()};
		convhull(values, ret.pixels());
		return ret;
	}

	inline void convhull(double_buffer<basic_image<float>>& buffers)
	{
		convhull(buffers.front(), buffers.back());
		buffers.swap();
	}
}

#endif