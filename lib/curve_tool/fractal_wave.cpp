//@	{"target": {"name": "fractal_wave.o"}}

#include "./fractal_wave.hpp"

std::vector<terraformer::location> terraformer::generate(fractal_wave const& wave_xy,
	wave_params const& wave_xy_params,
	fractal_wave const& wave_xz,
	wave_params const& wave_xz_params,
	uniform_polyline_params const& line_params)
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

	auto const target_amplitude_xy = wave_xy_params.amplitude;
	auto const target_amplitude_xz = wave_xz_params.amplitude;
	scaling const scaling{1.0f, target_amplitude_xy/wave_xy_amplitude, target_amplitude_xz/wave_xz_amplitude};

	std::vector<location> ret;
	ret.reserve(n_points);
	for(size_t k = 0; k != std::size(curve); ++k)
	{ ret.push_back(line_params.start_location + curve[k].apply(scaling)); }
	return ret;
}