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

bool is_ccw(location prevprev, location prev, location current)
{
	auto const dx_1 = prev.x - prevprev.x;
	auto const dy_1 = prev.y - prevprev.y;
	auto const dx_2 = current.x - prev.x;
	auto const dy_2 = current.y - prev.y;

	return dy_2/dx_2 > dy_1/dx_1;
}

std::vector<location> make_convhull(std::span<location const> loc)
{
	std::vector<location> buffer;

	for(size_t k = 0; k != std::size(loc); ++k)
	{
		while(std::size(buffer) >= 2)
		{
			auto const prevprev = *(std::end(buffer) - 2);
			auto const prev = *(std::end(buffer) - 1);
			if(!is_ccw(prevprev, prev, loc[k]))
			{ break; }

			buffer.pop_back();
		}

		buffer.push_back(loc[k]);
	}
	return buffer;
}

int main()
{
	terraformer::fractal_wave::params const wave_params{
		.wavelength = 64.0f,
		.per_wave_component_scaling_factor = std::numbers::phi_v<float>,
		.exponent_noise_amount = 0.0f,
		.per_wave_component_phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise_amount = 0.0f
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

	auto const convhull = make_convhull(vals);

	std::array<std::span<location const>, 2> vals_to_plot{vals, convhull};
	pretty::plot(vals_to_plot);

}