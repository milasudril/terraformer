#ifndef TERRAFORMER_UI_THEMING_WHSI_HPP
#define TERRAFORMER_UI_THEMING_WHSI_HPP

#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/math_utils/mod.hpp"
#include "lib/math_utils/cubic_spline.hpp"

namespace terraformer::ui::theming
{
	constexpr auto make_rgba_pixel_from_whsi(float hue, float saturation, float intensity, float alpha)
	{
		hue = mod(hue, 1.0f);

		hue = [](float val){
			constexpr std::array<cubic_spline_control_point, 4> hue_curve{
				cubic_spline_control_point{
					.y = 0.0f,
					.ddx = 0.25f
				},
				cubic_spline_control_point{
					.y = 2.0f,
					.ddx = 0.25f
				},
				cubic_spline_control_point{
					.y = 4.0f,
					.ddx = 0.25f
				},
				cubic_spline_control_point{
					.y = 6.0f,
					.ddx = 0.25f
				}
			};

			if(val < 1.0f/3.0f)
			{ return make_polynomial(hue_curve[0], hue_curve[1])(3.0f*val); }

			if(val < 2.0f/3.0f)
			{ return make_polynomial(hue_curve[1], hue_curve[2])(3.0f*(val - 1.0f/3.0f)); }

			return make_polynomial(hue_curve[2], hue_curve[3])(3.0f*(val - 2.0f/3.0f));
		}(hue);

		auto const z = 1.0f - std::abs(mod(hue, 2.0f) - 1.0f);
		auto const c = 3.0f*intensity*saturation/(1.0f + z);
		auto const x = c*z;

		auto const v_temp = [](float hue, float c, float x) {
			if(hue < 1.0f)
			{ return geosimd::vec_t{c, x, 0.0f, 0.0f}; }
			if(hue < 2.0f)
			{ return geosimd::vec_t{x, c, 0.0f, 0.0f}; }
			if(hue < 3.0f)
			{ return geosimd::vec_t{0.0f, c, x, 0.0f}; }
			if(hue < 4.0f)
			{ return geosimd::vec_t{0.0f, x, c, 0.0f}; }
			if(hue < 5.0f)
			{ return geosimd::vec_t{x, 0.0f, c, 0.0f}; }
			return geosimd::vec_t{c, 0.0f, x, 0.0f};
		}(hue, c, x);

		constexpr geosimd::vec_t weights{5.0f/16.0f, 1.0f/2.0f, 3.0f/16.0f, 0.0f};
		auto const weights_inv = geosimd::vec_t{1.0f, 1.0f, 1.0f, 0.0f}
				/(16.0f*weights + geosimd::vec_t{0.0f, 0.0f, 0.0f, 1.0f});
		auto const m = intensity*(1.0f - saturation)*weights + geosimd::vec_t{0.0f, 0.0f, 0.0f, alpha};
		return rgba_pixel{(v_temp + m)*weights_inv};
	}
}
#endif