//@	{"target":{"name":"ridge_curve.o"}}

#include "./ridge_curve.hpp"
#include "./composite_function.hpp"
#include "./first_order_hp_filter.hpp"
#include "./second_order_lp_filter.hpp"

#include <random>
#include <numbers>

std::vector<float> terraformer::generate(
	ridge_curve_description const& src,
	random_generator& rng,
	size_t seg_count,
	float dx)
{
	std::vector<float> ret(seg_count);
	constexpr auto twopi = 2.0f*std::numbers::pi_v<float>;
	std::uniform_real_distribution U{-1.0f, 1.0f};

	composite_function f{
		first_order_hp_filter{
			first_order_hp_filter_description{
				.cutoff_freq = twopi/src.wavelength,
				.initial_value = 0.0f,
				.initial_input = 0.0f
			},
			dx
		},
		second_order_lp_filter{
			second_order_lp_filter_description{
				.damping = src.damping,
				.cutoff_freq = twopi/src.wavelength,
				.initial_value = 0.0f,
				.initial_derivative = 0.0f,
				.initial_input = 0.0f
			},
			dx
		}
	};

	for(size_t k = 0; k != seg_count; ++k)
	{ ret[k] = f(U(rng)); }

	auto minmax = std::ranges::minmax_element(ret);

	auto const gain = 2.0f*src.amplitude/(*minmax.max - *minmax.min);

	for(size_t k = 0; k != seg_count; ++k)
	{ ret[k] *= gain; }

	return ret;
}