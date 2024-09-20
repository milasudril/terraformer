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
		{ return intensity(input, weights); }

		constexpr terraformer::rgba_pixel max_blue_compensate_with_other(float rg_factor, float target_intensity = 0.5f)
		{
			auto const b = 1.0f;
			auto const input_intensity = weights[2]*b;
			auto const leftover = target_intensity - input_intensity;
			auto const r = (1.0f - rg_factor)*leftover/weights[0];
			auto const g = rg_factor*leftover;

			return rgba_pixel{r, g, b, 0.0f};
		}

		constexpr rgba_pixel brighten(rgba_pixel x, float target_intensity = 0.5f)
		{
			auto const input_intensity = intensity(x);
			auto const white = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};
			auto const intensity_white = intensity(white);
			auto const t  = (target_intensity - input_intensity)/(intensity_white - input_intensity);

			return t*white + (1.0f - t)*x;

		//	return amount*rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f} + (1.0f - amount)*x;

		}

		constexpr rgba_pixel normalize_blend_white(rgba_pixel x, float target_intensity = 0.5f)
		{
			auto const tmp = x*target_intensity/intensity(x);

			auto const maxval = max_color_value(tmp);
			if(maxval > 1.0f)
			{
				auto const tmp_fullscale = tmp/maxval;
				// inner_product(t*white + (1 - t)*tmp_fullscale, weights) = target_intensity
				//
				// inner_product(t*white, weights) + inner_product((1 - t)*tmp_fullscale, weights) =
				// t*inner_product(white, weights) + (1 - t)*inner_product(tmp_fullscale, weights) =
				// t*(inner_product(white, weights) - inner_product(tmp_fullscale, weights))
				//      + inner_product(tmp_fullscale, weights)
				//
				// t*(intensity(white) - intensity(tmp_fullscale)) = target_intensity - intensity(tmp_fullscale)
				// t = (target_intensity - intensity(tmp_fullscale))/(intensity(white) - intensity(tmp_fullscale))
				//
				auto const input_intensity = intensity(tmp_fullscale);
				auto const white = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};
				auto const intensity_white = intensity(white);
				auto const t  = (target_intensity - input_intensity)/(intensity_white - input_intensity);

				return t*white + (1.0f - t)*tmp_fullscale;
			}
			return tmp;
		}

		constexpr auto generate_lut()
		{
			std::array ret{
				normalize_blend_white(terraformer::rgba_pixel{1.0f, 0.0f, 0.0f, 0.0f}),
				normalize_blend_white(terraformer::rgba_pixel{1.0f, 0.333f, 0.0f, 0.0f}),
				normalize_blend_white(terraformer::rgba_pixel{1.0f, 1.0f, 0.0f, 0.0f}),
				normalize_blend_white(terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 0.0f}),
				normalize_blend_white(terraformer::rgba_pixel{0.0f, 0.5f, 1.0f, 0.0f}),
				normalize_blend_white(terraformer::rgba_pixel{0.5f, 0.0f, 1.0f, 0.0f}),
			};

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