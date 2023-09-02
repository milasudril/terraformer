#ifndef TERRAFORMER_BUMP_FIELD_2_HPP
#define TERRAFORMER_BUMP_FIELD_2_HPP

#include "lib/curve_tool/fractal_wave.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/domain_size.hpp"

#include <algorithm>

namespace terraformer
{
	class bump_field_2
	{
	public:
		struct params{};

		float operator()(uint32_t x, uint32_t y) const
		{
			auto const u = m_coord_mapping(x, y).first - m_u_0;
			auto const v = m_coord_mapping(x, y).second;
			auto const x_amp_factor = std::exp2(-std::abs(u)/m_x_amp_half_length);
			auto const y_amp_factor = std::exp2(-std::abs(u)/m_x_amp_half_length);
			auto const x_wavelenth_factor = std::exp2(std::abs(u)/m_y_wavelength_half_length);
			auto const y_wavelenth_factor = std::exp2(std::abs(u)/m_y_wavelength_half_length);

			auto const wave_x = m_wave_x(v*x_wavelenth_factor/m_wave_x_params.wavelength + m_wave_x_params.phase);
			auto const wave_y = m_wave_y(u*y_wavelenth_factor/m_wave_y_params.wavelength + m_wave_y_params.phase);

			return std::lerp(x_amp_factor*wave_x, y_amp_factor*wave_y, m_xy_blend);
		}
	}

	template<class Rng>
	std::ranges::min_max_result<float> generate(span_2d<float> output_buffer,
		Rng&& rng,
		float pixel_size,
		span_2d<std::pair<float, float> const> coord_mapping,
		bump_field_2::params const& params)
	{
		return generate_minmax(output_buffer, bump_field{
			rng,
			domain_size{
				.width = output_buffer.width(),
				.height = output_buffer.height(),
				.pixel_size = pixel_size
			},
			cooord_mapping,
			params
		});
	}
}

#endif