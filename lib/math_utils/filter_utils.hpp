//@	{"dependencies_extra":[{"ref":"./filter_utils.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTER_UTILS_HPP
#define TERRAFORMER_FILTER_UTILS_HPP

#include "lib/common/span_2d.hpp"
#include "lib/math_utils/computation_context.hpp"
#include <complex>

namespace terraformer
{
	void make_filter_input(
		scanline_tranform_job const& jobinfo,
		span_2d<float const> input,
		span_2d<std::complex<float>> output
	);

	void make_filter_output(
		scanline_tranform_job const& jobinfo,
		span_2d<std::complex<float> const> input,
		span_2d<float> output
	);

	signaling_counter apply_filter(
		span_2d<float const> input,
		span_2d<float> filtered_output,
		computation_context& comp_ctxt,
		span_2d<float const> filter_mask
	);
}

#endif