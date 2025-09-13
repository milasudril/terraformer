//@	{"dependencies_extra":[{"ref":"./butter_bp_2d.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_BUTTER_BP_2D_HPP
#define TERRAFORMER_BUTTER_BP_2D_HPP

#include "lib/common/span_2d.hpp"
#include "lib/execution/signaling_counter.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/math_utils/computation_context.hpp"

namespace terraformer
{
	struct butter_bp_2d_descriptor
	{
		float f_x;
		float f_y;
		float lf_rolloff;
		float hf_rolloff;
		float y_direction;
	};


	signaling_counter apply(
		span_2d<float> output,
		butter_bp_2d_descriptor const&,
		span_2d<float const> input,
		computation_context& comp_ctxt
	);

	inline grayscale_image apply(
		butter_bp_2d_descriptor const& filter,
		span_2d<float const> input,
		computation_context& comp_ctxt
	)
	{
		auto const w = input.width();
		auto const h = input.height();
		grayscale_image filtered_output{w, h};
		apply(filtered_output, filter, input, comp_ctxt).wait();
		return filtered_output;
	}
}

#endif