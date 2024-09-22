#ifndef TERRAFORMER_UI_THEMING_COLOR_SCHEME_HPP
#define TERRAFORMER_UI_THEMING_COLOR_SCHEME_HPP

#include "./fixed_intensity_colormap.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

#include <array>

namespace terraformer::ui::theming
{
	struct color_set
	{
		rgba_pixel background;
		rgba_pixel text;
	};

	struct twocolor_gradient
	{
		rgba_pixel begin;
		rgba_pixel end;
	};

	struct color_scheme
	{
		color_set main_panel;
		color_set other_panel;
		color_set input_area;
		color_set command_area;
		color_set output_area;

		rgba_pixel cursor_color;
		rgba_pixel selection_color;
		rgba_pixel mouse_focus_color;
		rgba_pixel keyboard_focus_color;

		twocolor_gradient progress_colors;
		twocolor_gradient resource_usage_colors;

		std::array<rgba_pixel, 12> misc_dark_colors;
		std::array<rgba_pixel, 12> misc_mid_colors;
		std::array<rgba_pixel, 12> misc_bright_colors;
	};

	constexpr auto max_val = 1.0f;
	constexpr auto rate = (-1.0f - std::log2(max_val))/4.0f;
	static_assert(max_val*std::exp2(0.0f*rate) == max_val);
	static_assert(max_val*std::exp2(4.0f*rate) == 0.5f);

	template<size_t N, class Generator>
	constexpr auto sample_colormap(Generator&& gen)
	{
		std::array<rgba_pixel, N> ret{};

		for(size_t k = 0; k != std::size(ret); ++k)
		{ ret[k] = gen(static_cast<float>(k)/static_cast<float>(std::size(ret))); }

		return ret;
	}

	constexpr fixed_intensity_colormap mid_colors{perceptual_color_intensity{0.5f}};
	constexpr auto bright_colors = fixed_intensity_colormap::make_pastels(mid_colors, perceptual_color_intensity{0.75f});
	constexpr fixed_intensity_colormap dark_colors{perceptual_color_intensity{0.25f}};

	constexpr auto default_dark_palette = sample_colormap<12>(bright_colors);
	constexpr auto default_mid_palette = sample_colormap<12>(mid_colors);
	constexpr auto default_bright_palette = sample_colormap<12>(dark_colors);

	constexpr auto error_hue = 0.0f;
	constexpr auto warning_hue = 1.0f/6.0f;
	constexpr auto result_pending_hue = 1.0f/3.0f;
	constexpr auto ready_hue = 1.0f/2.0f;
	constexpr auto info_hue = 2.0f/3.0f;
	constexpr auto all_resources_busy_hue = 1.0f/12.0f;
	constexpr auto all_resources_free_hue = 1.0f/2.0f;

	constexpr auto default_dark_error_color = dark_colors(error_hue);
	constexpr auto default_mid_error_color = mid_colors(error_hue);
	constexpr auto default_bright_error_color = bright_colors(error_hue);

	constexpr auto default_dark_warning_color = dark_colors(warning_hue);
	constexpr auto default_mid_warning_color = mid_colors(warning_hue);
	constexpr auto default_bright_warning_color = bright_colors(warning_hue);

	constexpr auto default_dark_result_pending_color = dark_colors(result_pending_hue);
	constexpr auto default_mid_result_pending_color = mid_colors(result_pending_hue);
	constexpr auto default_bright_result_pending_color = bright_colors(result_pending_hue);

	constexpr auto default_dark_ready_color = dark_colors(ready_hue);
	constexpr auto default_mid_ready_color = mid_colors(ready_hue);
	constexpr auto default_bright_ready_color = bright_colors(ready_hue);

	constexpr auto default_dark_info_color = dark_colors(info_hue);
	constexpr auto default_mid_info_color = mid_colors(info_hue);
	constexpr auto default_bright_info_color = bright_colors(info_hue);

	constexpr auto default_dark_all_resources_busy_color = dark_colors(all_resources_busy_hue);
	constexpr auto default_mid_all_resources_busy_color = mid_colors(all_resources_busy_hue);
	constexpr auto default_bright_all_resources_busy_color = bright_colors(all_resources_busy_hue);

	constexpr auto default_dark_all_resources_free_color = dark_colors(all_resources_free_hue);
	constexpr auto default_mid_all_resources_free_color = mid_colors(all_resources_free_hue);
	constexpr auto default_bright_all_resources_free_color = bright_colors(all_resources_free_hue);

	constexpr color_scheme default_color_scheme{
		.main_panel{
			.background = rgba_pixel{
				std::exp2(rate*3.0f),
				std::exp2(rate*3.0f),
				std::exp2(rate*3.0f),
				1.0f
			},
			.text = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			}
		},
		.other_panel{
			.background = rgba_pixel{
				std::exp2(rate*4.0f),
				std::exp2(rate*4.0f),
				std::exp2(rate*4.0f),
				1.0f
			},
			.text = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			}
		},
		.input_area{
			.background = rgba_pixel{
				std::exp2(rate*0.0f),
				std::exp2(rate*0.0f),
				std::exp2(rate*0.0f),
				1.0f
			},
			.text = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			}
		},
		.command_area{
			.background = rgba_pixel{
				std::exp2(rate*1.0f),
				std::exp2(rate*1.0f),
				std::exp2(rate*1.0f),
				1.0f
			},
			.text = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			}
		},
		.output_area{
			.background = rgba_pixel{
				std::exp2(rate*2.0f),
				std::exp2(rate*2.0f),
				std::exp2(rate*2.0f),
				1.0f
			},
			.text = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			}
		},
		.cursor_color = mid_colors(1.0f/3.0f),
		.selection_color = default_bright_warning_color,
		.mouse_focus_color = mid_colors(11.0f/12.0f),
		.keyboard_focus_color = mid_colors(9.0f/12.0f),
		.progress_colors{
			.begin = default_mid_result_pending_color,
			.end = default_mid_ready_color
		},
		.resource_usage_colors{
			.begin = default_mid_all_resources_free_color,
			.end = default_mid_all_resources_busy_color
		},
		.misc_dark_colors = default_dark_palette,
		.misc_mid_colors = default_mid_palette,
		.misc_bright_colors = default_bright_palette
	};
}

#endif