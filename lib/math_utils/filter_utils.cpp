//@	{"target":{"name":"filter_utils.o"}}

#include "./filter_utils.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/utils.hpp"
#include "lib/execution/signaling_counter.hpp"
#include "lib/common/chunk_by_chunk_count_view.hpp"
#include "lib/pixel_store/image.hpp"

#include <cassert>
#include <ranges>

void terraformer::make_filter_input(
	span_2d<float const> input,
	uint32_t y_input_offset,
	span_2d<std::complex<float>> output
)
{
	auto const w = output.width();
	auto const h = output.height();
	auto sign_y = (y_input_offset%2 == 0)? 1.0f : -1.0f;
	for(uint32_t y = 0; y != h; ++y)
	{
		auto sign_x = 1.0f;
		for(uint32_t x = 0; x != w; ++x)
		{
			output(x, y) = input(x, y + y_input_offset) * sign_y * sign_x;
			sign_x *= -1.0f;
		}
		sign_y *= -1.0f;
	}
}

void terraformer::make_filter_output(
	span_2d<std::complex<float> const> input,
	uint32_t y_input_offset,
	span_2d<float> output
)
{
	auto const w = output.width();
	auto const h = output.height();
	auto sign_y = (y_input_offset%2 == 0)? 1.0f : -1.0f;
	for(uint32_t y = 0; y != h; ++y)
	{
		auto sign_x = 1.0f;
		for(uint32_t x = 0; x != w; ++x)
		{
			output(x, y) = input(x, y + y_input_offset).real() * sign_x * sign_y;
			sign_x *= -1.0f;
		}
		sign_y *= -1.0f;
	}
}

terraformer::signaling_counter terraformer::apply_filter(
	span_2d<float const> input,
	span_2d<float> filtered_output,
	computation_context& comp_ctxt,
	span_2d<float const> filter_mask
)
{
	auto const w = input.width();
	auto const h = input.height();

	terraformer::basic_image<std::complex<float>> filter_input{w, h};
	dispatch_jobs(
		input,
		filter_input.pixels(),
		comp_ctxt.workers,
		[]<class ... Args>(auto, Args&&... args){
			make_filter_input(std::forward<Args>(args)...);
		}
	).wait();

	terraformer::basic_image<std::complex<float>> transformed_input{w, h};
	comp_ctxt.dft_engine.transform(
		std::as_const(filter_input).pixels(),
		transformed_input.pixels(),
		dft_direction::forward
	).wait();

	dispatch_jobs(
		filter_mask,
		transformed_input.pixels(),
		comp_ctxt.workers,
		[]<class ... Args>(auto, Args&&... args){
			multiply_assign(std::forward<Args>(args)...);
		}
	).wait();

	comp_ctxt.dft_engine.transform(
		std::as_const(transformed_input).pixels(),
		filter_input.pixels(),
		dft_direction::backward
	).wait();

	return dispatch_jobs(
		std::as_const(filter_input).pixels(),
		filtered_output,
		comp_ctxt.workers,
		[]<class ... Args>(auto, Args&&... args){
			make_filter_output(std::forward<Args>(args)...);
		}
	);
}