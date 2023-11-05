#ifndef TERRAFORMER_FILTERED_NOISE_GENERATOR_HPP
#define TERRAFORMER_FILTERED_NOISE_GENERATOR_HPP

#include "lib/filters/dft_engine.hpp"
#include "lib/common/utils.hpp"

#include <random>
#include <algorithm>
#include <memory>
#include <span>

namespace terraformer
{
	struct filtered_noise_description
	{
		float lambda_0;
		float hp_order;
		float lp_order;
		float dx;
		size_t point_count;
	};

	template<class Rng>
	std::vector<float> generate(Rng&& rng, filtered_noise_description const& params)
	{
		auto const point_count = params.point_count;
		auto const signal_length = 2*point_count;
		auto const dx = params.dx;
		auto const lambda_max = dx*static_cast<float>(signal_length);
		auto const lambda_0 = params.lambda_0;
		auto const hp_order = params.hp_order;
		auto const lp_order = params.lp_order;

		auto filter = std::make_unique_for_overwrite<float[]>(signal_length);
		std::uniform_real_distribution U{0.0f, 1.0f};
		for(size_t k = 1; k != signal_length/2; ++k)
		{
			auto const omega = static_cast<float>(k);
			auto const xi = omega*lambda_0/lambda_max;
			auto const amp = 1.0f/
				std::sqrt((1.0f + std::pow(xi, -2.0f*hp_order)) * (1.0f + std::pow(xi, 2.0f*lp_order)));
			filter[k] = amp;
			filter[signal_length - k] = amp;
		}

		auto noise = std::make_unique_for_overwrite<std::complex<float>[]>(signal_length);
		for(size_t k = 0; k != signal_length; ++k)
		{ noise[k] = U(rng); }

		auto transformed_signal = std::make_unique_for_overwrite<std::complex<float>[]>(signal_length);
		get_plan(signal_length, dft_direction::forward)
			.execute(noise.get(), transformed_signal.get());

		for(size_t k = 0; k != signal_length; ++k)
		{ transformed_signal[k] *= filter[k]; }

		get_plan(signal_length, dft_direction::backward).execute(transformed_signal.get(), noise.get());

		std::span const output_vals{noise.get(), signal_length};
		auto range = std::ranges::minmax_element(output_vals, [](auto a, auto b) {
			return a.real() < b.real();
		});
		auto const amplitude = 0.5f*(*range.max - *range.min).real();
		auto const offset = 0.5f*(*range.max + *range.min).real();
		auto const max_index = range.max - std::begin(output_vals);

		std::vector<float> ret(point_count);
		for(size_t k = 0; k != point_count; ++k)
		{
			auto const src_index = (k + max_index + signal_length - point_count/2)%signal_length;
			auto const src_val = output_vals[src_index].real();
			ret[k] = (src_val - offset)/amplitude;
		}

		return ret;
	}
}

#endif