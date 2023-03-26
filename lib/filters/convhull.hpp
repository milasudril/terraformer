//@	{"dependencies_extra":[{"ref":"./convhull.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTERS_CONVHULL_HPP
#define TERRAFORMER_FILTERS_CONVHULL_HPP

#include "lib/common/span_2d.hpp"
#include "lib/pixel_store/image.hpp"

#include <span>
#include <vector>

namespace terraformer
{
	[[nodiscard]] std::vector<float> convhull(std::span<float const> values);
}

#endif