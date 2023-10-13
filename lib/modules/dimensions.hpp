#ifndef TERRAFORMER_DIMENSIONS_HPP
#define TERRAFORMER_DIMENSIONS_HPP

#include "lib/common/bounded_value.hpp"
#include "lib/common/interval.hpp"

namespace terraformer
{
	using domain_length = bounded_value<open_open_interval{0.0f, 131072.0f}, 49152.0f>;
	using elevation = bounded_value<closed_closed_interval{-16384.0f, 16384.0f}, 768.0f>;
	using pixel_count = bounded_value<closed_closed_interval{9, 67'108'864}, 1'048'576>;
}

#endif