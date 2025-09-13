//@	{"dependencies_extra":[{"ref":"./filter_utils.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTER_UTILS_HPP
#define TERRAFORMER_FILTER_UTILS_HPP

#include "lib/common/span_2d.hpp"
#include "lib/execution/signaling_counter.hpp"
#include "lib/pixel_store/image.hpp"
#include <complex>

namespace terraformer
{
	void make_filter_input(
		span_2d<std::complex<float>> output,
		span_2d<float const> input,
		uint32_t y_input_offset
	);

	terraformer::basic_image<std::complex<float>> make_filter_input(span_2d<float const> input);
}

#endif