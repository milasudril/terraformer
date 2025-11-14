//@	{"target":{"name":"ridge_curve.o"}}

#include "./ridge_curve.hpp"

#include "lib/math_utils/first_order_hp_filter.hpp"
#include "lib/math_utils/second_order_lp_filter.hpp"
#include "lib/math_utils/composite_function.hpp"
#include "lib/math_utils/cubic_spline.hpp"

#include <random>
#include <numbers>

terraformer::single_array<float> terraformer::generate(
	ridge_tree_branch_displacement_description const& src,
	random_generator& rng,
	array_size<float> seg_count,
	float dx,
	float warmup_periods
)
{
	single_array<float> ret;
	ret.resize(seg_count);
	constexpr auto twopi = 2.0f*std::numbers::pi_v<float>;
	auto const noise_cutoff = std::sqrt(2.0f)*twopi/src.wavelength;
	std::uniform_real_distribution U{-1.0f, 1.0f};

	composite_function f{
		[
			filter = first_order_hp_filter{
				first_order_hp_filter_description{
					.cutoff_freq = noise_cutoff,
					.initial_value = 0.0f,
					.initial_input = 0.f
				}
			},
			dx
		](float x) mutable {
			return filter(x, dx);
		},
		[
			filter = second_order_lp_filter{
				second_order_lp_filter_description{
					.damping = src.damping,
					.cutoff_freq = noise_cutoff,
					.initial_value = 0.0f,
					.initial_derivative = 0.0f,
					.initial_input = 0.0f
				}
			},
			dx
		](float x) mutable{
			return filter(x, dx);
		}
	};

	auto warmup_count = static_cast<size_t>(src.wavelength*warmup_periods);
	while(warmup_count != 0)
	{
		f(U(rng));
		--warmup_count;
	}

	auto envelope = [attack_length = src.attack_length.value_or(src.wavelength)](float x){
		auto const t = x/(attack_length);
		return t < 1.0f ?
			interp(
				cubic_spline_control_point{
					.y = 0.0f,
					.ddx = 0.0f
				},
				cubic_spline_control_point{
					.y = 1.0f,
					.ddx = 0.0f
				}, t) :
			1.0f;
	};

	for(auto k : ret.element_indices())
	{ ret[k] = f(U(rng)); }

	auto const minmax = std::ranges::minmax_element(ret);
	auto const min = *minmax.min;
	auto const max = *minmax.max;

	// TODO: C++23 adjacent_view
	for(auto k : ret.element_indices())
	{
		auto const input_val = (ret[k] - min)/(max - min);
		auto const x = static_cast<float>(k.get());
		ret[k] =
			 src.amplitude
			*input_val
			*std::sin(twopi*x*dx/src.wavelength)
			*envelope(dx*x)
			*envelope(dx*static_cast<float>(seg_count.get() - k.get()));
	}

	return ret;
}