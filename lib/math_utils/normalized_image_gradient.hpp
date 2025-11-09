//@	{"dependencies_extra":[{"ref":"./normalized_image_gradient.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_NORMALIZED_IMAGE_GRADIENT_HPP
#define TERRAFORMER_NORMALIZED_IMAGE_GRADIENT_HPP

#include "./computation_context.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/execution/signaling_counter.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree_branch.hpp"
#include "lib/pixel_store/image.hpp"

namespace terraformer
{
	void compute_normalized_gradient(
		scanline_processing_job_info const& jobinfo,
		span_2d<direction> output,
		span_2d<float const> input
	);

	inline batch_result<void> compute_normalized_gradient(
		span_2d<direction> output,
		thread_pool<move_only_function<void()>>& workers,
		span_2d<float const> input
	)
	{
		return process_scanlines(
			output,
			workers,
			[]<class... Args>(Args&&...args){
				compute_normalized_gradient(std::forward<Args>(args)...);
			},
			input
		);
	}
}

#endif