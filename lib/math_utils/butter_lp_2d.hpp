//@	{"dependencies_extra":[{"ref":"./butter_lp_2d.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_BUTTER_LP_2D_HPP
#define TERRAFORMER_BUTTER_LP_2D_HPP

#include "./computation_context.hpp"
#include "./filter_utils.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/execution/signaling_counter.hpp"
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

	void make_filter_mask(
		scanline_processing_job_info const& jobinfo,
		span_2d<float> output,
		butter_lp_2d_descriptor const& params
	);

	inline batch_result<void> apply(
		span_2d<float const> input,
		span_2d<float> filtered_output,
		computation_context& comp_ctxt,
		butter_lp_2d_descriptor const& params
	)
	{
		auto const w = input.width();
		auto const h = input.height();

		terraformer::basic_image<float> filter_mask{w, h};
		process_scanlines(
			filter_mask.pixels(),
			comp_ctxt.workers,
			[]<class ... Args>(Args&&... params){
				make_filter_mask(std::forward<Args>(params)...);
			},
			params
		).wait();

		return apply_filter(
			input, filtered_output, comp_ctxt, std::as_const(filter_mask).pixels()
		);
	}

	inline grayscale_image apply(
		butter_lp_2d_descriptor const& filter,
		span_2d<float const> input,
		computation_context& comp_ctxt
	)
	{
		auto const w = input.width();
		auto const h = input.height();
		grayscale_image filtered_output{w, h};
		apply(input, filtered_output.pixels(), comp_ctxt, filter).wait();
		return filtered_output;
	}

}



#endif