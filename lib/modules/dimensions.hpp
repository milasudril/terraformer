#ifndef TERRAFORMER_DIMENSIONS_HPP
#define TERRAFORMER_DIMENSIONS_HPP

#include "lib/common/bounded_value.hpp"
#include "lib/common/interval.hpp"

namespace terraformer
{
	using domain_length = bounded_value<closed_closed_interval{1.0f, 131072.0f}, 49152.0f>;
	using horizontal_amplitude = bounded_value<closed_closed_interval{1.0f, 65536.0f}, 1.0f>;
	using elevation = bounded_value<closed_closed_interval{-16384.0f, 16384.0f}, 768.0f>;
	using vertical_amplitude = bounded_value<closed_closed_interval{1.0f, 8192.0f}, 1.0f>;
	using pixel_count = bounded_value<closed_closed_interval{9, 67'108'864}, 1'048'576>;
	using global_phase = bounded_value<closed_closed_interval{-8.0f, 8.0f}, 0.0f>;
	using scaling_factor = bounded_value<closed_closed_interval{1.0f, 8.0f}, 1.0f>;
	using filter_order = bounded_value<closed_closed_interval{1.0f, 8.0f}, 1.0f>;
	using noise_amplitude = bounded_value<closed_closed_interval{0.0f, 1.0f}, 0.0f>;
	using phase_offset = bounded_value<closed_closed_interval{-0.5f, 0.5f}, 0.0f>;
	using modulation_depth = bounded_value<closed_closed_interval{0.0f, 1.0f}, 0.0f>;

	using blend_value = bounded_value<closed_closed_interval{0.0f, 1.0f}, 0.5f>;
}

#endif