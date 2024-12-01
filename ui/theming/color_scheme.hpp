#ifndef TERRAFORMER_UI_THEMING_COLOR_SCHEME_HPP
#define TERRAFORMER_UI_THEMING_COLOR_SCHEME_HPP

#include "./fixed_intensity_colormap.hpp"
#include "ui/main/config.hpp"

namespace terraformer::ui::theming
{

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
	constexpr auto bright_colors = fixed_intensity_colormap::make_pastels(mid_colors, perceptual_color_intensity{1.0f});
	constexpr fixed_intensity_colormap dark_colors{perceptual_color_intensity{0.25f}};

	constexpr auto default_dark_palette = sample_colormap<12>(dark_colors);
	constexpr auto default_mid_palette = sample_colormap<12>(mid_colors);
	constexpr auto default_bright_palette = sample_colormap<12>(bright_colors);

	constexpr auto error_hue = 0.0f;
	constexpr auto warning_hue = 1.0f/4.0f;
	constexpr auto result_pending_hue = 1.0f/3.0f;
	constexpr auto ok_hue = 1.0f/2.0f;
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

	constexpr auto default_dark_ok_color = dark_colors(ok_hue);
	constexpr auto default_mid_ok_color = mid_colors(ok_hue);
	constexpr auto default_bright_ok_color = bright_colors(ok_hue);

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

	constexpr main::color_scheme default_color_scheme{
		.main_panel{
			.background = rgba_pixel{
				std::exp2(rate*3.0f),
				std::exp2(rate*3.0f),
				std::exp2(rate*3.0f),
				1.0f
			}
		},
		.other_panel{
			.background = rgba_pixel{
				std::exp2(rate*4.0f),
				std::exp2(rate*4.0f),
				std::exp2(rate*4.0f),
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
			.foreground = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			},
			.selection = default_mid_warning_color
		},
		.command_area{
			.background = rgba_pixel{
				std::exp2(rate*1.0f),
				std::exp2(rate*1.0f),
				std::exp2(rate*1.0f),
				1.0f
			},
			.foreground = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			},
			.selection = default_mid_warning_color
		},
		.output_area{
			.background = rgba_pixel{
				std::exp2(rate*2.0f),
				std::exp2(rate*2.0f),
				std::exp2(rate*2.0f),
				1.0f
			},
			.foreground = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			},
			.selection = default_mid_warning_color
		},
		.status_indicator{
			.progress_meter{
				.begin = default_mid_result_pending_color,
				.end = default_mid_ready_color
			},
			.resource_usage_meter{
				.begin = default_mid_all_resources_free_color,
				.end = default_mid_all_resources_busy_color
			},
			.error_indication = default_mid_error_color,
			.warning_indication = default_mid_warning_color,
			.progress_indication = default_mid_result_pending_color,
			.ok_indication = default_mid_ready_color,
			.info_indication = default_mid_info_color
		},
		.mouse_kbd_tracking{
			.cursor_color = mid_colors(1.0f/3.0f),
			.mouse_focus_color = mid_colors(9.0f/12.0f),
			.keyboard_focus_color = mid_colors(11.0f/12.0f)
		},
		.misc_colors{
			.dark = default_dark_palette,
			.mid = default_mid_palette,
			.bright = default_bright_palette
		}
	};
}

#endif