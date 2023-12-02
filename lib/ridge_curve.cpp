//@	{"target":{"name":"ridge_curve.o"}}

#include "./ridge_curve.hpp"
#include "./composite_function.hpp"
#include "./first_order_hp_filter.hpp"
#include "./second_order_lp_filter.hpp"

#include <random>
#include <numbers>

std::vector<terraformer::displacement> terraformer::generate(
	ridge_curve_description const& src,
	random_generator& rng,
	size_t seg_count,
	float dx)
{
	std::vector<displacement> ret(seg_count);
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
				.damping = std::sqrt(0.5f),
				.cutoff_freq = twopi/src.wavelength,
				.initial_value = 0.0f,
				.initial_derivative = 0.0f,
				.initial_input = 0.0f
			},
			dx
		}
	};

	for(size_t k = 0; k != seg_count; ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		ret[k] = displacement{x, f(U(rng)), 0.0f};
	}

	return ret;
}