//@	{"target":{"name":"filtered_noise_generator.o"}}

#include "./filtered_noise_generator.hpp"

void terraformer::apply_filter(std::span<float const> input,
	float* output,
	float lambda_max,
	filtered_noise_description_1d const& params)
{
	auto const signal_length = std::size(input);
	auto const lambda_0 = params.lambda_0;
	auto const hp_order = params.hp_order;
	auto const lp_order = params.lp_order;

	auto filter = std::make_unique<float[]>(signal_length);
	std::uniform_real_distribution U{0.0f, 1.0f};
	for(size_t k = 1; k != signal_length/2; ++k)
	{
		auto const omega = static_cast<float>(k);
		auto const xi = std::min(omega*lambda_0/lambda_max, 16.0f);
		auto const amp = 1.0f/
			std::sqrt((1.0f + std::pow(xi, -2.0f*hp_order)) * (1.0f + std::pow(xi, 2.0f*lp_order)));
		filter[k] = amp;
		filter[signal_length - k] = amp;
	}

	auto complex_signal = std::make_unique_for_overwrite<std::complex<float>[]>(signal_length);
	std::ranges::copy(input, complex_signal.get());

	auto transformed_signal = std::make_unique<std::complex<float>[]>(signal_length);
	get_plan(signal_length, dft_direction::forward)
		.execute(complex_signal.get(), transformed_signal.get());

	for(size_t k = 0; k != signal_length; ++k)
	{ transformed_signal[k] *= filter[k]; }

	get_plan(signal_length, dft_direction::backward).execute(transformed_signal.get(), complex_signal.get());

	std::span const output_vals{complex_signal.get(), signal_length};

	auto range = std::ranges::minmax_element(output_vals, [](auto a, auto b) {
		return a.real() < b.real();
	});
	auto const amplitude = 0.5f*(*range.max - *range.min).real();
	auto const offset = 0.5f*(*range.max + *range.min).real();
	auto const max_index = range.max - std::begin(output_vals);

	for(size_t k = max_index; k != signal_length + max_index; ++k)
	{
		auto const src_index = k%signal_length;
		auto const src_val = output_vals[src_index].real();
		output[k - max_index] = (src_val - offset)/amplitude;
	}
}

std::vector<terraformer::location> terraformer::generate(
	filtered_noise_generator_1d const& wave_xy,
	output_range xy_output_range,
	filtered_noise_generator_1d const& wave_xz,
	output_range xz_output_range,
	polyline_location_params const& line_params)
{
	auto const n_points = line_params.point_count;
	auto const dx = line_params.dx;
	auto const phase_xy = 0.0f;
	auto const phase_xz = 0.0f;

	std::vector<displacement> curve;
	curve.reserve(n_points);
	for(size_t k = 0; k != n_points; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const y = wave_xy(x + phase_xy);
		auto const z = wave_xz(x + phase_xz);
		curve.push_back(displacement{x, y, z});
	}

	auto const xy_amp_out = 0.5f*(xy_output_range.max - xy_output_range.min);
	auto const xy_offset_out = 0.5f*(xy_output_range.min + xy_output_range.max);

	auto const xz_amp_out = 0.5f*(xz_output_range.max - xz_output_range.min);
	auto const xz_offset_out = 0.5f*(xz_output_range.min + xz_output_range.max);

	scaling const scaling{1.0f, xy_amp_out, xz_amp_out};
	displacement const offset_out{0.0f, xy_offset_out, xz_offset_out};

	std::vector<location> ret;
	ret.reserve(n_points);
	for(size_t k = 0; k != std::size(curve); ++k)
	{ ret.push_back(line_params.start_location + curve[k].apply(scaling) + offset_out); }
	return ret;
}