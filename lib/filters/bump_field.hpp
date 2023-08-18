//@	{"dependencies_extra":[{"ref":"./bump_field.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_LIB_BUMP_FIELD_HPP
#define TERRAFORMER_LIB_BUMP_FIELD_HPP

#include "lib/curve_tool/fractal_wave.hpp"
#include "lib/common/span_2d.hpp"

#include <algorithm>

namespace terraformer
{
	class bump_field
	{
	public:
		struct params
		{
			float pixel_size;
			fractal_wave_params impact_waves;
			fractal_wave_params x_distortion;
			fractal_wave_params y_distortion;
		};

		template<class Rng>
		explicit bump_field(Rng&& rng, params const& params):
			m_pixel_size{params.pixel_size},
			m_x_distortion{generate(rng, params.x_distortion)},
			m_y_distortion{generate(rng, params.y_distortion)},
			m_wave{rng, params.impact_waves.shape},
			m_wave_params{params.impact_waves.wave_properties}
		{}

		float operator()(uint32_t x, uint32_t y) const
		{
			auto const xf = m_pixel_size*static_cast<float>(x);
			auto const yf = m_pixel_size*static_cast<float>(y);
			auto const current_loc =
				  location{m_x_distortion[y][1], m_y_distortion[x][1], 0.0f}
				+ displacement{xf, yf, 0.0f};
			auto convsum = 0.0f;
			for(size_t k = 0; k != std::size(m_ridge_curve); ++k)
			{
				auto const d = distance_xy(current_loc, m_ridge_curve[k]);
				convsum += m_wave(d/m_wave_params.wavelength + m_wave_params.phase);
			}
			return convsum;
		}

		float amplitude() const
		{ return m_wave_params.amplitude; }

	private:
		float m_pixel_size;
		std::vector<location> m_x_distortion;
		std::vector<location> m_y_distortion;
		std::span<location const> m_ridge_curve;
		fractal_wave m_wave;
		wave_params m_wave_params;
	};

	std::ranges::min_max_result<float> generate(span_2d<float> output_buffer, bump_field const& bumps);
}

#endif