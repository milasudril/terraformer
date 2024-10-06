#ifndef TERRAFORMER_UI_THEMING_HPP
#define TERRAFORMER_UI_THEMING_HPP

#include "lib/pixel_store/intensity.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include <array>

namespace terraformer::ui::theming
{
	using perceptual_color_intensity = terraformer::intensity<0.5673828125f, 1.0f, 0.060546875f>;

	class fixed_intensity_colormap
	{
	public:
		constexpr explicit fixed_intensity_colormap(perceptual_color_intensity intensity):
			m_lut{
				normalize(basic_colors[0], intensity),
				normalize(basic_colors[1], intensity),
				normalize(basic_colors[2], intensity),
				normalize(basic_colors[3], intensity),
				normalize(basic_colors[4], intensity),
				normalize(basic_colors[5], intensity),
			}
		{}

		static constexpr auto make_pastels(
			fixed_intensity_colormap const& src,
			perceptual_color_intensity intensity
		)
		{
			fixed_intensity_colormap ret{};
			for(size_t k = 0; k != std::size(basic_colors); ++k)
			{ ret.m_lut[k] = brighten(src.m_lut[k], intensity); }

			return ret;
		}

		constexpr rgba_pixel operator()(float t) const
		{ return interp(m_lut, static_cast<float>(std::size(m_lut))*t, wrap_around_at_boundary{}) + rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f}; }

	private:
		fixed_intensity_colormap() = default;

		static constexpr std::array basic_colors{
			terraformer::rgba_pixel{1.0f, 0.0f, 0.0f, 0.0f},
			terraformer::rgba_pixel{1.0f, 0.333f, 0.0f, 0.0f},
			terraformer::rgba_pixel{1.0f, 1.0f, 0.0f, 0.0f},
			terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 0.0f},
			terraformer::rgba_pixel{0.0f, 0.25f, 1.0f, 0.0f},
			terraformer::rgba_pixel{0.333f, 0.0f, 1.0f, 0.0f},
		};

		std::array<terraformer::rgba_pixel, std::size(basic_colors)> m_lut;
	};
}
#endif