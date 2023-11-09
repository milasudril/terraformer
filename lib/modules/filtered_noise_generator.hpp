//@	{"dependencies_extra":[{"ref":"./filtered_noise_generator.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTERED_NOISE_GENERATOR_HPP
#define TERRAFORMER_FILTERED_NOISE_GENERATOR_HPP

#include "lib/filters/dft_engine.hpp"
#include "lib/common/utils.hpp"
#include "lib/interp.hpp"
#include "lib/boundary_sampling_policies.hpp"

#include <random>
#include <algorithm>
#include <memory>
#include <span>

namespace terraformer
{
	struct filtered_noise_description_1d
	{
		float lambda_0;
		float hp_order;
		float lp_order;
		float peak_offset;
	};

	void apply_filter(std::span<float const> input, float* output, float lambda_max, filtered_noise_description_1d const& params);

	class filtered_noise_generator_1d
	{
	public:
		template<class Rng>
		explicit filtered_noise_generator_1d(Rng&& rng,
			size_t point_count,
			float dx,
			filtered_noise_description_1d const& params):
			m_signal_length{2*point_count},
			m_signal{std::make_unique<float[]>(m_signal_length)},
			m_dx{dx}
		{
			std::uniform_real_distribution U{0.0f, 1.0f};
			auto noise = std::make_unique_for_overwrite<float[]>(m_signal_length);
			for(size_t k = 0; k != m_signal_length; ++k)
			{ noise[k] = U(rng); }
			apply_filter(std::span{noise.get(), m_signal_length}, m_signal.get(), static_cast<float>(point_count)*dx, params);
		}

		float operator()(float x) const
		{ return interp(std::span{m_signal.get(), m_signal_length}, x/m_dx, wrap_around_at_boundary{}); }

		float dx() const { return m_dx; }

	private:
		size_t m_signal_length;
		std::unique_ptr<float[]> m_signal;
		float m_dx;
	};
}

#endif