#ifndef TERRAFORMER_UI_THEMING_COLOR_SCHEME_HPP
#define TERRAFORMER_UI_THEMING_COLOR_SCHEME_HPP

#include "./whi.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

#include <array>

namespace terraformer::ui::theming
{
	struct color_set
	{
		rgba_pixel background;
		rgba_pixel text;
		rgba_pixel border;
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
		std::array<rgba_pixel, 12> misc_bright_colors;
	};

	constexpr auto max_val = 1.0f;
	constexpr auto rate = (-1.0f - std::log2(max_val))/4.0f;
	static_assert(max_val*std::exp2(0.0f*rate) == max_val);
	static_assert(max_val*std::exp2(4.0f*rate) == 0.5f);

	template<class Generator>
	constexpr auto generate_default_palette(Generator&& gen)
	{
		std::array<rgba_pixel, 12> ret{};

		for(size_t k = 0; k != std::size(ret); ++k)
		{
			auto const hue = static_cast<float>((7*k)%std::size(ret))/static_cast<float>(std::size(ret));

			ret[k] = gen(hue, 1.0f);
		}

		return ret;
	}

	constexpr auto default_dark_palette = generate_default_palette(make_rgba_pixel_from_whi);
	constexpr auto default_bright_palette = generate_default_palette(make_rgba_pixel_from_whi_inv);

	constexpr auto error_hue = 0.0f;
	constexpr auto warning_hue = 1.0f/6.0f;
	constexpr auto worker_busy_hue = 0.0f;
	constexpr auto worker_ready_hue = 1.0f/3.0f;

	constexpr auto default_dark_error_color = make_rgba_pixel_from_whi(error_hue, 1.0f);
	constexpr auto default_bright_error_color = make_rgba_pixel_from_whi(error_hue, 1.0f);
	constexpr auto default_dark_warning_color = make_rgba_pixel_from_whi(warning_hue, 1.0f);
	constexpr auto default_bright_warning_color = make_rgba_pixel_from_whi_inv(warning_hue, 1.0f);
	constexpr auto default_dark_worker_busy_color = make_rgba_pixel_from_whi(worker_busy_hue, 1.0f);
	constexpr auto default_bright_worker_busy_color = make_rgba_pixel_from_whi_inv(worker_busy_hue, 1.0f);
	constexpr auto default_dark_worker_ready_color = make_rgba_pixel_from_whi(worker_ready_hue, 1.0f);
	constexpr auto default_bright_worker_ready_color = make_rgba_pixel_from_whi_inv(worker_ready_hue, 1.0f);
	constexpr auto default_dark_resources_free_color = default_dark_worker_ready_color;
	constexpr auto default_bright_resources_free_color = default_bright_worker_ready_color;
	constexpr auto default_dark_resources_used_color = default_dark_worker_busy_color;
	constexpr auto default_bright_resources_used_color = default_bright_worker_busy_color;

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
			},
			.border = rgba_pixel{
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
			},
			.border = rgba_pixel{
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
			},
			.border = rgba_pixel{
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
			},
			.border = rgba_pixel{
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
			},
			.border = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			}
		},
		.cursor_color = make_rgba_pixel_from_whi(2.0f/3.0f, 1.0f),
		.selection_color = default_bright_warning_color,
		.mouse_focus_color = make_rgba_pixel_from_whi(3.0f/4.0f, 1.0f),
		.keyboard_focus_color = make_rgba_pixel_from_whi(7.0f/12.0f, 1.0f),
		.progress_colors{
			.begin = default_dark_worker_busy_color,
			.end = default_dark_worker_ready_color
		},
		.resource_usage_colors{
			.begin = default_dark_resources_free_color,
			.end = default_dark_resources_used_color
		},
		.misc_dark_colors = default_dark_palette,
		.misc_bright_colors = default_bright_palette
	};
}

#endif