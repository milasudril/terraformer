//@	{"target":{"name":"./butter_bp_2d.o"}}

#include "./butter_bp_2d.hpp"
#include "./filter_utils.hpp"
#include "./dft_engine.hpp"
#include "lib/common/span_2d.hpp"

#include <cassert>


void terraformer::make_filter_mask(
	uint32_t input_height,
	uint32_t input_y_offset,
	terraformer::span_2d<float> output,
	butter_bp_2d_descriptor const& params
)
{
	auto const w = output.width();
	auto const h = output.height();

	auto const f_x = params.f_x;
	auto const f_y = params.f_y;
	auto const w_float = static_cast<float>(w);
	auto const h_float = static_cast<float>(input_height);
	auto const x_0 = 0.5f*w_float;
	auto const y_0 = 0.5f*h_float;
	auto const cos_theta = std::cos(params.y_direction);
	auto const sin_theta = std::sin(params.y_direction);
	auto const hf_rolloff = params.hf_rolloff;
	auto const lf_rolloff = params.lf_rolloff;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const xi_in = static_cast<float>(x) - x_0;
			auto const eta_in = static_cast<float>(y + input_y_offset) - y_0;

			auto const xi = (xi_in*cos_theta + eta_in*sin_theta)/f_x;
			auto const eta = (-xi_in*sin_theta + eta_in*cos_theta)/f_y;

			auto const r2 = xi*xi + eta*eta;

			auto const lpf = 1.0f/std::sqrt(1.0f + std::pow(r2, hf_rolloff));
			auto const hpf = std::pow(r2, 0.5f*lf_rolloff)/std::sqrt(1.0f + std::pow(r2, lf_rolloff));
			output(x, y) = 2.0f*lpf*hpf;
		}
	}
}

terraformer::signaling_counter terraformer::apply(
	span_2d<float> filtered_output,
	butter_bp_2d_descriptor const& params,
	span_2d<float const> input,
	computation_context& comp_ctxt
)
{
	signaling_counter counter{0};
	auto const w = input.width();
	auto const h = input.height();

	terraformer::basic_image<float> filter_mask{w, h};
	auto pending_mask = dispatch_jobs(filter_mask.pixels(), comp_ctxt.workers, make_filter_mask, params);

	terraformer::basic_image<std::complex<float>> filter_input{w, h};
	dispatch_jobs(input, filter_input.pixels(), comp_ctxt.workers, make_filter_input).wait();

	terraformer::basic_image<std::complex<float>> transformed_input{w, h};
	auto pending_transform = comp_ctxt.dft_engine.transform(
		std::as_const(filter_input).pixels(),
		transformed_input.pixels(),
		dft_direction::forward
	);

	pending_mask.wait();
	pending_transform.wait();
	dispatch_jobs(
		std::as_const(filter_mask).pixels(),
		transformed_input.pixels(),
		comp_ctxt.workers,
		multiply_assign<float const, std::complex<float>>
	).wait();

	comp_ctxt.dft_engine.transform(
		std::as_const(transformed_input).pixels(),
		filter_input.pixels(),
		dft_direction::backward
	).wait();

	{
		auto sign_y = 1.0f;
		for(uint32_t y = 0; y < h; ++y)
		{
			auto sign_x = 1.0f;
			for(uint32_t x = 0; x < w; ++x)
			{
				filtered_output(x, y) = filter_input(x, y).real() * sign_x * sign_y;
				sign_x *= -1.0f;
			}
			sign_y *= -1.0f;
		}
	}

	return counter;
}
