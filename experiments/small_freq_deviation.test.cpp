#include "../lib/curve_tool/wave_sum.hpp"

#include <pretty/plot.hpp>
#include <ranges>

struct location
{
	double x;
	double y;
};

template<size_t k>
auto get(location);

template<>
auto get<0>(location loc){ return loc.x; }

template<>
auto get<1>(location loc){ return loc.y; }

template<>
struct std::tuple_size<location>
{
	static constexpr size_t value = 2;
};

int main()
{
	terraformer::wave_sum_params const wave_params{
		.base_frequency = 1.0/41000.0,
		.frequency_ratio = std::sqrt(2)/std::sqrt(3),
		.phase_difference = 0.0,
		.mix = 0.25
	};

	terraformer::wave_sum const wave{wave_params};

	pretty::print(1.0/(wave_params.base_frequency*wave_params.frequency_ratio));

	std::array<location, 2048> vals;
	auto const dt = 1e6/static_cast<double>(std::size(vals));
	for(size_t k = 0; k != std::size(vals); ++k)
	{
		auto const t = dt*static_cast<double>(k);
		vals[k] = location{t, 50000.0*wave(t)};
	}

	std::array<std::span<location const>, 1> vals_to_plot{vals};
	pretty::plot(vals_to_plot);
}