//@	{"dependencies_extra":[{"ref":"./butter_lp_2d.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_BUTTER_LP_2D_HPP
#define TERRAFORMER_BUTTER_LP_2D_HPP

#include "lib/pixel_store/image.hpp"
namespace terraformer
{
	struct butter_lp_2d_descriptor
	{
		float f_x;
		float f_y;
		float hf_rolloff;
		float y_direction;
	};

	grayscale_image apply(butter_lp_2d_descriptor const&, span_2d<float const> input);
}

#endif