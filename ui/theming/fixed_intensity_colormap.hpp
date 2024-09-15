//@	{"dependencies_extra":[{"ref":"./fixed_intensity_colormap.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_THEMING_HPP
#define TERRAFORMER_UI_THEMING_HPP

#include "lib/pixel_store/rgba_pixel.hpp"

#include <array>

namespace terraformer::ui::theming
{
	class fixed_intensity_colormap
	{
	public:
		static constexpr terraformer::rgba_pixel::storage_type const weights{
			0.5673828125f,
			1.0f,
			0.060546875f,
			0.0f
		};

		static constexpr auto get_max_intensity()
		{
			auto const maxvals = rgba_pixel::storage_type{1.0f, 1.0f, 1.0f, 0.0f}*weights;
			return (maxvals[0] + maxvals[1] + maxvals[2])/3.0f;
		}

		static constexpr terraformer::rgba_pixel max_blue_compensate_with_other(float rg_factor)
		{
			auto const b = 1.0f;
			auto const input_intensity = weights[2]*b;
			auto const leftover = get_max_intensity() - input_intensity;
			auto const r = (1.0f - rg_factor)*leftover/weights[0];
			auto const g = rg_factor*leftover;

			return rgba_pixel{r, g, b, 0.0f};
		}

		static constexpr rgba_pixel normalize(rgba_pixel x)
		{
			auto const vec = x.value();
			auto const scaled_vals = vec*weights;
			auto const input_intensity = scaled_vals[0] + scaled_vals[1] + scaled_vals[2];
			return x*get_max_intensity()/input_intensity;
		}

	 rgba_pixel operator()(float t) const;

	 static std::array<rgba_pixel, 7> const& get_lut();
	};
}
#endif