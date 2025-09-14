//@	{"target":{"name":"./butter_bp_2d.o"}}

#include "./butter_bp_2d.hpp"

void terraformer::make_filter_mask(
	scanline_generate_job const& jobinfo,
	terraformer::span_2d<float> output,
	butter_bp_2d_descriptor const& params
)
{
	auto const w = output.width();
	auto const h = output.height();

	auto const f_x = params.f_x;
	auto const f_y = params.f_y;
	auto const w_float = static_cast<float>(w);
	auto const h_float = static_cast<float>(jobinfo.total_height);
	auto const input_y_offset = jobinfo.input_y_offset;
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