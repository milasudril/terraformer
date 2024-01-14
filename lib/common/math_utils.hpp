#ifndef TERRAFORMER_MATHUTILS_HPP
#define TERRAFORMER_MATHUTILS_HPP

#include <numbers>
#include <cmath>

namespace terraformer
{
	template<std::floating_point Precision>
	struct wave_sum_params
	{
		Precision base_frequency;
		Precision frequency_ratio;
		Precision phase_difference;
		Precision mix;
	};

	template<std::floating_point Precision>
	class wave_sum
	{
	public:
		using params = wave_sum_params<Precision>;

		explicit wave_sum(params const& params): m_params{params}
		{
		}

		auto operator()(Precision x) const
		{
			auto const twopi = static_cast<Precision>(2.0)*std::numbers::pi_v<float>;
			auto const f_0 = m_params.base_frequency;
			auto const f_1 = f_0*m_params.frequency_ratio;
 			return std::lerp(std::cos(twopi*f_0*x),
				std::cos(twopi*(f_1*x + m_params.phase_difference)), m_params.mix);
		}

	private:
		params m_params;
	};
}

#endif