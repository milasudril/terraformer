//@	{"target": {"name": "fractal_wave.o"}}

#include "./fractal_wave.hpp"

std::vector<terraformer::displacement> terraformer::generate(fractal_wave const& wave,
	wave_params const& wave_params,
	output_range output_range,
	polyline_displacement_params const& line_params)
{
	auto const n_points = line_params.point_count;
	auto const dx = line_params.dx;
	auto const wavelength = wave_params.wavelength;
	auto const phase = wave_params.phase;

	std::vector<displacement> curve;
	curve.reserve(n_points);
	float wave_amplitude = 0.0f;
	for(size_t k = 0; k != n_points; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const y = wave(x/wavelength + phase);
		auto const z = 0.0f;
		curve.push_back(displacement{x, y, z});
		wave_amplitude = std::max(std::abs(wave_amplitude), y);
	}

	auto const amplitude = 0.5f*(output_range.max - output_range.min);
	displacement const dc_offset{0.0f, 0.5f*(output_range.min + output_range.max), 0.0f};
	scaling const scaling{1.0f, amplitude/wave_amplitude, 1.0f};
	for(size_t k = 0; k != std::size(curve); ++k)
	{
		curve[k].apply(scaling);
		curve[k] += dc_offset;
	}

	return curve;
}

std::vector<terraformer::location> terraformer::generate(fractal_wave const& wave,
	wave_params const& wave_params,
	output_range output_range,
	polyline_location_params const& line_params)
{
	auto const n_points = line_params.point_count;
	auto const dx = line_params.dx;
	auto const wavelength = wave_params.wavelength;
	auto const phase = wave_params.phase;

	std::vector<displacement> curve;
	curve.reserve(n_points);
	float wave_amplitude = 0.0f;
	for(size_t k = 0; k != n_points; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const y = wave(x/wavelength + phase);
		auto const z = 0.0f;
		curve.push_back(displacement{x, y, z});
		wave_amplitude = std::max(std::abs(wave_amplitude), y);
	}

	auto const amplitude = 0.5f*(output_range.max - output_range.min);
	displacement const dc_offset{0.0f, 0.5f*(output_range.min + output_range.max), 0.0f};
	scaling const scaling{1.0f, amplitude/wave_amplitude, 1.0f};

	std::vector<location> ret;
	ret.reserve(n_points);
	for(size_t k = 0; k != std::size(curve); ++k)
	{ ret.push_back(line_params.start_location + curve[k].apply(scaling) + dc_offset); }
	return ret;
}

std::vector<terraformer::location> terraformer::generate(fractal_wave const& wave_xy,
	wave_params const& wave_xy_params,
	output_range xy_output_range,
	fractal_wave const& wave_xz,
	wave_params const& wave_xz_params,
	output_range xz_output_range,
	polyline_location_params const& line_params)
{
	auto const n_points = line_params.point_count;
	auto const dx = line_params.dx;
	auto const wavelength_xy = wave_xy_params.wavelength;
	auto const phase_xy = wave_xy_params.phase;
	auto const wavelength_xz = wave_xz_params.wavelength;
	auto const phase_xz = wave_xz_params.phase;

	std::vector<displacement> curve;
	curve.reserve(n_points);
	float wave_xy_amplitude = 0.0f;
	float wave_xz_amplitude = 0.0f;
	for(size_t k = 0; k != n_points; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const y = wave_xy(x/wavelength_xy + phase_xy);
		auto const z = wave_xz(x/wavelength_xz + phase_xz);
		curve.push_back(displacement{x, y, z});
		wave_xy_amplitude = std::max(std::abs(wave_xy_amplitude), y);
		wave_xz_amplitude = std::max(std::abs(wave_xz_amplitude), z);
	}

	auto const xy_amp = 0.5f*(xy_output_range.max - xy_output_range.min);
	auto const xy_dc_offset = 0.5f*(xy_output_range.min + xy_output_range.max);

	auto const xz_amp = 0.5f*(xz_output_range.max - xz_output_range.min);
	auto const xz_dc_offset = 0.5f*(xz_output_range.min + xz_output_range.max);

	scaling const scaling{1.0f, xy_amp/wave_xy_amplitude, xz_amp/wave_xz_amplitude};
	displacement const dc_offset{0.0f, xy_dc_offset, xz_dc_offset};

	std::vector<location> ret;
	ret.reserve(n_points);
	for(size_t k = 0; k != std::size(curve); ++k)
	{ ret.push_back(line_params.start_location + curve[k].apply(scaling) + dc_offset); }
	return ret;
}