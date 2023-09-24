#ifndef TERRAFORMER_BUMP_FIELD_2_HPP
#define TERRAFORMER_BUMP_FIELD_2_HPP

#include "./waveshaper.hpp"

#include "lib/modules/fractal_wave.hpp"
#include "lib/common/span_2d.hpp"

#include <algorithm>
#include <utility>

namespace terraformer
{
	struct wave_scaling
	{
		float amp_half_length;
		float wavelength_half_length;
	};

	class bump_field_2
	{
	public:
		struct params
		{
			wave_scaling x_scale;
			wave_scaling y_scale;
			fractal_wave_description x_wave;
			fractal_wave_description y_wave;
			float xy_blend;
		};

		template<class Rng>
		explicit bump_field_2(Rng&& rng,
			span_2d<std::pair<float, float> const> coord_mapping,
			float u_0,
			float v_0,
			params const& params):
			m_coord_mapping{coord_mapping},
			m_u_0{u_0},
			m_v_0{v_0},
			m_x_scale{params.x_scale},
			m_y_scale{params.y_scale},
			m_x_wave{rng, params.x_wave.shape},
			m_x_wave_params{params.x_wave.wave_properties},
			m_y_wave{rng, params.y_wave.shape},
			m_y_wave_params{params.y_wave.wave_properties},
			m_xy_blend{params.xy_blend}
		{}


		float operator()(uint32_t x, uint32_t y) const
		{
			auto const u = m_coord_mapping(x, y).first - m_u_0;
			auto const v = m_coord_mapping(x, y).second - m_v_0;

			auto const x_amp_factor = std::exp2(-std::abs(u)/m_x_scale.amp_half_length);
			auto const x_wavelenth_factor = std::exp2(std::abs(u)/m_x_scale.wavelength_half_length);
			auto const y_amp_factor = std::exp2(-std::abs(u)/m_y_scale.amp_half_length);
			auto const y_wavelenth_factor = std::exp2(std::abs(u)/m_y_scale.wavelength_half_length);

			auto const y_wave_val = y_amp_factor*m_y_wave(u*y_wavelenth_factor/m_y_wave_params.wavelength + m_y_wave_params.phase);
			auto const x_wave_val = x_amp_factor*m_x_wave(v*x_wavelenth_factor/m_x_wave_params.wavelength + m_x_wave_params.phase);

			return std::lerp(x_wave_val, y_wave_val, m_xy_blend);
		}

	private:
		span_2d<std::pair<float, float> const> m_coord_mapping;
		float m_u_0;
		float m_v_0;
		wave_scaling m_x_scale;
		wave_scaling m_y_scale;
		fractal_wave m_x_wave;
		wave_params m_x_wave_params;
		fractal_wave m_y_wave;
		wave_params m_y_wave_params;
		float m_xy_blend;
	};

	template<class Rng>
	void generate(span_2d<float> output_buffer,
		Rng&& rng,
		span_2d<std::pair<float, float> const> coord_mapping,
		float u_0,
		float v_0,
		output_range output_range,
		bump_field_2::params const& params)
	{
		auto const input_range = generate_minmax(output_buffer, bump_field_2{
			rng,
			coord_mapping,
			u_0,
			v_0,
			params
		});
		normalize(output_buffer, input_range, output_range);
	}
}

#endif