//@	{"dependencies_extra":[{"ref":"./butter_bp_2d.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_BUTTER_BP_2D_HPP
#define TERRAFORMER_BUTTER_BP_2D_HPP

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

	grayscale_image apply(
		butter_bp_2d_descriptor const&,
		span_2d<float const> input,
		computation_context& comp_ctxt
	);
}

#endif