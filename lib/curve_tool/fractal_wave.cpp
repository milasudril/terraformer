//@	{"target": {"name": "fractal_wave.o"}}

#include "./fractal_wave.hpp"

std::vector<terraformer::location> terraformer::generate(fractal_wave const& wave,
	wave_params const& wave_params,
	uniform_polyline_params const& line_params,
	rotation const& R)
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
		curve.push_back(displacement{x, y, 0.0f});
		wave_amplitude = std::max(std::abs(wave_amplitude), y);
	}

	auto const target_amplitude = wave_params.amplitude;
	scaling const scaling{1.0f, target_amplitude/wave_amplitude, 1.0f};

	std::vector<location> ret;
	ret.reserve(n_points);
	for(size_t k = 0; k != std::size(curve); ++k)
	{ ret.push_back(line_params.start_location + curve[k].apply(scaling).apply(R)); }
	return ret;
}