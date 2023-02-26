#include "../lib/curve_tool/fractal_wave.hpp"

#include <pretty/plot.hpp>
#include <random>
#include <ranges>

using random_generator = std::mt19937;

struct location
{
	float x;
	float y;
};

template<size_t k>
float get(location);

template<>
float get<0>(location loc){ return loc.x; }

template<>
float get<1>(location loc){ return loc.y; }

template<>
struct std::tuple_size<location>
{
	static constexpr size_t value = 2;
};

int main()
{
	terraformer::fractal_wave::params const wave_params{
		.wavelength = 16.0f,
		.per_wave_component_scaling_factor = 1.0f + 1.0f/128.0f,
		.exponent_noise_amount = 1.0f/(128.0f*16.0f),
		.per_wave_component_phase_shift = 0.0f,
		.phase_shift_noise_amount = 1.0f/12.0f
	};

	random_generator rng;
	terraformer::fractal_wave wave{rng, 0.0f, wave_params};

	std::array<location, 128> vals;
	for(size_t k = 0; k != std::size(vals); ++k)
	{
		vals[k] = location{static_cast<float>(k), wave(static_cast<float>(k))};
	}

	for(size_t k = 0; k != std::size(vals);++k)
	{
		vals[k].y *= 16.0f/wave.amplitude();
	}

	std::array<std::span<location const>, 1> vals_to_plot{vals};
	pretty::plot(vals_to_plot);

}