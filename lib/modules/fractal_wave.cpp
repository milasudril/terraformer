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
	auto min =  std::numeric_limits<float>::infinity();
	auto max = -std::numeric_limits<float>::infinity();
	for(size_t k = 0; k != n_points; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const y = wave(x/wavelength + phase);
		auto const z = 0.0f;
		curve.push_back(displacement{x, y, z});
		min = std::min(y, min);
		max = std::max(y, max);
	}

	displacement const offset_in{0.0f, 0.5f*(max + min), 0.0f};
	auto const amp_in = 0.5f*(max - min);

	displacement const offset_out{0.0f, 0.5f*(output_range.min + output_range.max), 0.0f};
	auto const amp_out = 0.5f*(output_range.max - output_range.min);

	scaling const scaling{1.0f, amp_out/amp_in, 1.0f};
	for(size_t k = 0; k != std::size(curve); ++k)
	{
		curve[k] -= offset_in;
		curve[k].apply(scaling);
		curve[k] += offset_out;
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
	auto min =  std::numeric_limits<float>::infinity();
	auto max = -std::numeric_limits<float>::infinity();
	for(size_t k = 0; k != n_points; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const y = wave(x/wavelength + phase);
		auto const z = 0.0f;
		curve.push_back(displacement{x, y, z});
		min = std::min(y, min);
		max = std::max(y, max);
	}

	displacement const offset_in{0.0f, 0.5f*(max + min), 0.0f};
	auto const amp_in = 0.5f*(max - min);

	displacement const offset_out{0.0f, 0.5f*(output_range.min + output_range.max), 0.0f};
	auto const amp_out = 0.5f*(output_range.max - output_range.min);

	scaling const scaling{1.0f, amp_out/amp_in, 1.0f};

	std::vector<location> ret;
	ret.reserve(n_points);
	for(size_t k = 0; k != std::size(curve); ++k)
	{ ret.push_back(line_params.start_location + (curve[k] - offset_in).apply(scaling) + offset_out); }
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
	auto min_y =  std::numeric_limits<float>::infinity();
	auto max_y = -std::numeric_limits<float>::infinity();
	auto min_z =  std::numeric_limits<float>::infinity();
	auto max_z = -std::numeric_limits<float>::infinity();
	for(size_t k = 0; k != n_points; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const y = wave_xy(x/wavelength_xy + phase_xy);
		auto const z = wave_xz(x/wavelength_xz + phase_xz);
		curve.push_back(displacement{x, y, z});
		min_y = std::min(min_y, y);
		min_z = std::min(min_z, z);
		max_y = std::max(max_y, y);
		max_z = std::max(max_z, z);
	}

	auto const xy_amp_in = 0.5f*(max_y - min_y);
	auto const xy_offset_in = 0.5f*(max_y + min_y);
	auto const xy_amp_out = 0.5f*(xy_output_range.max - xy_output_range.min);
	auto const xy_offset_out = 0.5f*(xy_output_range.min + xy_output_range.max);

	auto const xz_amp_in = 0.5f*(max_z - min_z);
	auto const xz_offset_in = 0.5f*(max_z + min_z);
	auto const xz_amp_out = 0.5f*(xz_output_range.max - xz_output_range.min);
	auto const xz_offset_out = 0.5f*(xz_output_range.min + xz_output_range.max);

	scaling const scaling{1.0f, xy_amp_out/xy_amp_in, xz_amp_out/xz_amp_in};
	displacement const offset_in{0.0f, xy_offset_in, xz_offset_in};
	displacement const offset_out{0.0f, xy_offset_out, xz_offset_out};

	std::vector<location> ret;
	ret.reserve(n_points);
	for(size_t k = 0; k != std::size(curve); ++k)
	{ ret.push_back(line_params.start_location + (curve[k] - offset_in).apply(scaling) + offset_out); }
	return ret;
}