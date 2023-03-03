#ifndef TERRAFORMER_LIB_WAVESUM_HPP
#define TERRAFORMER_LIB_WAVESUM_HPP

#include <numbers>
#include <cmath>

namespace terraformer
{
	template<std::floating_point Precision>
	class wave_sum
	{
	public:
		struct params
		{
			Precision amplitude;
			Precision base_frequency;
			Precision frequency_ratio;
			Precision phase_difference;
			Precision mix;
		};

		explicit wave_sum(params const& params): m_params{params}
		{
		}

		auto operator()(Precision x) const
		{
			auto const twopi = static_cast<Precision>(2.0)*std::numbers::pi_v<float>;
			auto const f_0 = twopi*m_params.base_frequency;
			auto const f_1 = f_0*m_params.frequency_ratio;
 			return m_params.amplitude*std::lerp(std::cos(f_0*x),
				std::cos(f_1*x + m_params.phase_difference), m_params.mix);
		}

	private:
		params m_params;
	};
}

#endif