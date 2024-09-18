#ifndef TERRAFORMER_UI_THEMING_HPP
#define TERRAFORMER_UI_THEMING_HPP

#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include <array>

namespace terraformer::ui::theming
{
	namespace fixed_intensity_colormap_helpers
	{
		constexpr terraformer::rgba_pixel::storage_type const weights{
			0.5673828125f,
			1.0f,
			0.060546875f,
			0.0f
		};

		constexpr auto intensity(rgba_pixel input)
		{
			auto const vec = input.value();
			auto const scaled_vals = vec*weights;
			return scaled_vals[0] + scaled_vals[1] + scaled_vals[2];
		}

		constexpr terraformer::rgba_pixel max_blue_compensate_with_other(float rg_factor, float target_intensity = 0.5f)
		{
			auto const b = 1.0f;
			auto const input_intensity = weights[2]*b;
			auto const leftover = target_intensity - input_intensity;
			auto const r = (1.0f - rg_factor)*leftover/weights[0];
			auto const g = rg_factor*leftover;

			return rgba_pixel{r, g, b, 0.0f};
		}

		constexpr rgba_pixel normalize(rgba_pixel x, float target_intensity = 0.5f)
		{
			return x*target_intensity/intensity(x);
		}

		constexpr auto generate_lut()
		{
			std::array ret{
				normalize(terraformer::rgba_pixel{1.0f, 0.0f, 0.0f, 0.0f}),
				terraformer::rgba_pixel{},
				terraformer::rgba_pixel{},
				normalize(terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 0.0f}),
				max_blue_compensate_with_other(0.66667f),
				max_blue_compensate_with_other(0.33333f)
			};
			ret[2] = 0.36f*ret[0] + 0.64f*ret[3];
			ret[1] = 0.5f*ret[2] + 0.5f*ret[0];

			for(size_t k = 0; k != std::size(ret); ++k)
			{
				ret[k] = terraformer::rgba_pixel{ret[k].red(), ret[k].green(), ret[k].blue()};
			}

			return ret;
		}
	}

	class fixed_intensity_colormap
	{
	public:
		rgba_pixel operator()(float t) const
		{ return interp(lut, static_cast<float>(std::size(lut))*t, wrap_around_at_boundary{}); }

		static constexpr auto lut = fixed_intensity_colormap_helpers::generate_lut();
	};
}
#endif