#ifndef TERRAFORMER_BUMP_FIELD_HPP
#define TERRAFORMER_BUMP_FIELD_HPP

#include "lib/curve_tool/fractal_wave.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/domain_size.hpp"

#include <algorithm>

namespace terraformer
{
	class bump_field
	{
	public:
		struct params
		{
			fractal_wave_params impact_waves;
			fractal_wave_params x_distortion;
			fractal_wave_params y_distortion;
			float half_length;
		};

		template<class Rng>
		explicit bump_field(Rng&& rng,
			domain_size const& dom_size,
			std::span<location const> ridge_curve,
			params const& params):
			m_pixel_size{dom_size.pixel_size},
			m_ridge_curve{ridge_curve},
			m_wave{rng, params.impact_waves.shape},
			m_wave_params{params.impact_waves.wave_properties},
			m_half_length{params.half_length},
			m_x_distortion{generate(rng, params.x_distortion, polyline_location_params{
				.point_count = dom_size.height,
				.dx = dom_size.pixel_size,
				.start_location = location{0.0f, 0.0f, 0.0f}
			})},
			m_y_distortion{generate(rng, params.y_distortion, polyline_location_params{
				.point_count = dom_size.width,
				.dx = dom_size.pixel_size,
				.start_location = location{0.0f, 0.0f, 0.0f}
			})}
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
				auto const factor = std::exp2(-d/m_half_length);
				auto const factorl = std::exp2(-d/(2.0f*m_half_length));
				convsum += factor*m_wave(d/(factorl*m_wave_params.wavelength) + m_wave_params.phase);
			}
			return convsum;
		}

		float amplitude() const
		{ return m_wave_params.amplitude; }

	private:
		float m_pixel_size;
		std::span<location const> m_ridge_curve;
		fractal_wave m_wave;
		wave_params m_wave_params;
		float m_half_length;
		std::vector<location> m_x_distortion;
		std::vector<location> m_y_distortion;
	};

	template<class Rng>
	std::ranges::min_max_result<float> generate(span_2d<float> output_buffer,
		Rng&& rng,
		float pixel_size,
		std::span<location const> ridge_curve,
		bump_field::params const& params)
	{
		return generate_minmax(output_buffer, bump_field{
			rng,
			domain_size{
				.width = output_buffer.width(),
				.height = output_buffer.height(),
				.pixel_size = pixel_size
			},
			ridge_curve,
			params
		});
	}
}

#endif