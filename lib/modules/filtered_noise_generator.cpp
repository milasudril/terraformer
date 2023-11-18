//@	{"target":{"name":"filtered_noise_generator.o"}}

#include "./filtered_noise_generator.hpp"
#include "lib/pixel_store/image.hpp"

void terraformer::apply_filter(
	std::span<float const> input,
	std::span<std::complex<double>> output,
	double lambda_max,
	filtered_noise_description_1d const& params)
{
	auto const signal_length = std::size(input);
	auto const wavelength = static_cast<double>(params.wavelength);
	auto const hp_order = static_cast<double>(params.hp_order);
	auto const lp_order = static_cast<double>(params.lp_order);

	auto filter = std::make_unique<double[]>(signal_length);
	for(size_t k = 1; k != signal_length/2; ++k)
	{
		auto const omega = static_cast<double>(k);
		auto const xi = omega*wavelength/lambda_max;
		auto const amp = 1.0/
			std::sqrt((1.0 + std::pow(xi, -2.0*hp_order)) * (1.0 + std::pow(xi, 2.0*lp_order)));
		filter[k] = amp;
		filter[signal_length - k] = amp;
	}

	std::ranges::copy(input, output.data());

	auto transformed_signal = std::make_unique<std::complex<double>[]>(signal_length);
	get_plan(signal_length, dft_direction::forward)
		.execute(output.data(), transformed_signal.get());

	for(size_t k = 0; k != signal_length; ++k)
	{ transformed_signal[k] *= filter[k]; }

	get_plan(signal_length, dft_direction::backward).execute(transformed_signal.get(), output.data());
}

void terraformer::apply_filter(std::span<float const> input,
	float* output,
	double lambda_max,
	filtered_noise_description_1d const& params)
{
	auto const signal_length = std::size(input);
	auto complex_signal = std::make_unique_for_overwrite<std::complex<double>[]>(signal_length);
	apply_filter(input, std::span{complex_signal.get(), signal_length}, lambda_max, params);

	std::span const output_vals{complex_signal.get(), signal_length};

	auto range = std::ranges::minmax_element(output_vals, [](auto a, auto b) {
		return a.real() < b.real();
	});
	auto const amplitude = 0.5*(*range.max - *range.min).real();
	auto const offset = 0.5*(*range.max + *range.min).real();
	auto const max_index = range.max - std::begin(output_vals);

	for(size_t k = max_index; k != signal_length + max_index; ++k)
	{
		auto const src_index = k%signal_length;
		auto const src_val = output_vals[src_index].real();
		output[k - max_index] = static_cast<float>((src_val - offset)/amplitude);
	}
}

std::vector<terraformer::location> terraformer::generate(
	filtered_noise_generator_1d const& wave_xy,
	float amp_xy,
	float peak_loc_xy,
	bool flip_x_xy,
	bool flip_displacement_xy,
	filtered_noise_generator_1d const& wave_xz,
	float amp_xz,
	float peak_loc_xz,
	bool flip_x_xz,
	bool flip_displacement_xz,
	polyline_location_params const& line_params)
{
	auto const n_points = line_params.point_count;
	auto const dx = line_params.dx;

	std::vector<displacement> curve;
	curve.reserve(n_points);
	auto const xy_x = flip_x_xy? -1.0f : 1.0f;
	amp_xy = flip_displacement_xy? -amp_xy:amp_xy;
	auto const xz_x = flip_x_xz? -1.0f : 1.0f;
	amp_xz = flip_displacement_xz? -amp_xz:amp_xz;
	for(size_t k = 0; k != n_points; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const y = amp_xy*wave_xy(xy_x*(x - peak_loc_xy));
		auto const z = amp_xz*wave_xz(xz_x*(x - peak_loc_xz));
		curve.push_back(displacement{x, y, z});
	}

	std::vector<location> ret;
	ret.reserve(n_points);
	for(size_t k = 0; k != std::size(curve); ++k)
	{ ret.push_back(line_params.start_location + curve[k]); }
	return ret;
}

void terraformer::apply_filter(
	span_2d<float const> input,
	span_2d<std::complex<double>> output,
	double lambda_max,
	filtered_noise_description_2d const& params)
{
	auto const w = input.width();
	auto const h = input.height();

	auto const wavelength_x = static_cast<double>(params.wavelength_x);
	auto const wavelength_y = static_cast<double>(params.wavelength_y);
	auto const hp_order = static_cast<double>(params.hp_order);
	auto const lp_order = static_cast<double>(params.lp_order);

	basic_image<double> filter{w, h};
	for(uint32_t y = 0; y != h/2; ++y)
	{
		for(uint32_t x = 0; x != w/2; ++x)
		{
			if(x == 0 && y == 0)
			{ filter(x, y) = 0.0f; }
			else
			{
				auto const omega_x = static_cast<double>(x);
				auto const omega_y = static_cast<double>(y);
				auto const xi_x = omega_x*wavelength_y/lambda_max;
				auto const xi_y = omega_y*wavelength_x/lambda_max;
				auto const xi = xi_x*xi_x + xi_y*xi_y;
				auto const amp = 1.0/
					std::sqrt((1.0 + std::pow(xi, -hp_order)) * (1.0 + std::pow(xi, lp_order)));
				filter(x, y) = amp;
				if(x != 0)
				{ filter(w - x, y) = amp; }
				if(y != 0)
				{ filter(x, h - y) = amp; }
				if(x != 0 && y != 0)
				{ filter(w - x, h - y) = amp; }
			}
		}
	}

	auto const signal_length = static_cast<size_t>(w)*static_cast<size_t>(h);
	std::copy_n(input.data(), signal_length, output.data());

	basic_image<std::complex<double>> transformed_image{w, h};
	get_plan(span_2d_extents{w, h}, dft_direction::forward)
		.execute(output.data(), transformed_image.pixels().data());

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{ transformed_image(x, y) *= filter(x, y); }
	}

	get_plan(span_2d_extents{w, h}, dft_direction::backward)
		.execute(transformed_image.pixels().data(), output.data());
}

void terraformer::apply_filter(span_2d<float const> input,
	span_2d<float> output,
	double lambda_max,
	filtered_noise_description_2d const& params)
{
	auto const w = input.width();
	auto const h = input.height();
	basic_image<std::complex<double>> complex_image{w, h};
	apply_filter(input, complex_image.pixels(), lambda_max, params);

	auto range = minmax_element(complex_image.pixels(), [](auto a, auto b) {
		return a.real() < b.real();
	});

	auto const minval = complex_image(range.min.x, range.min.y);
	auto const maxval = complex_image(range.max.x, range.max.y);

	auto const amplitude = 0.5f*(maxval - minval).real();
	auto const offset = 0.5f*(maxval + minval).real();

	for(uint32_t y = range.max.y; y != h + range.max.y; ++y)
	{
		for(uint32_t x = range.max.x; x != w + range.max.x; ++x)
		{
			auto const src_y = y % h;
			auto const src_x = x % w;
			auto const src_val = complex_image(src_x, src_y).real();
			output(x - range.max.x, y - range.max.y) = static_cast<float>((src_val - offset)/amplitude);
		}
	}
}