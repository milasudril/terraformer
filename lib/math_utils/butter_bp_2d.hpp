//@	{"dependencies_extra":[{"ref":"./butter_bp_2d.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_BUTTER_BP_2D_HPP
#define TERRAFORMER_BUTTER_BP_2D_HPP

#include "./computation_context.hpp"
#include "./filter_utils.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/execution/signaling_counter.hpp"
#include "lib/pixel_store/image.hpp"

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

	void make_filter_mask(
		uint32_t input_height,
		uint32_t input_y_offset,
		span_2d<float> output,
		butter_bp_2d_descriptor const& params
	);

	inline signaling_counter apply(
		span_2d<float const> input,
		span_2d<float> filtered_output,
		computation_context& comp_ctxt,
		butter_bp_2d_descriptor const& params
	)
	{
		auto const w = input.width();
		auto const h = input.height();

		terraformer::basic_image<float> filter_mask{w, h};
		dispatch_jobs(
			filter_mask.pixels(),
			comp_ctxt.workers,
			[]<class ... Args>(Args&&... params){
				make_filter_mask(std::forward<Args>(params)...);
			},
			params
		).wait();

		return apply_filter(
			input, filtered_output, comp_ctxt, filter_mask.pixels()
		);
	}

	inline grayscale_image apply(
		butter_bp_2d_descriptor const& filter,
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