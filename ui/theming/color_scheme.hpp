#ifndef TERRAFORMER_UI_THEMING_COLOR_SCHEME_HPP
#define TERRAFORMER_UI_THEMING_COLOR_SCHEME_HPP

#include "./whsi.hpp"
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

	struct color_scheme
	{
		color_set main_panel;
		color_set other_panel;
		color_set input_area;
		color_set command_area;
		color_set output_area;

		rgba_pixel selection_color;
		rgba_pixel mouse_focus_color;
		rgba_pixel keyboard_focus_color;

		std::array<rgba_pixel, 16> misc_colors;
	};

	constexpr auto max_val = 1.0f;
	constexpr auto rate = (-1.0f - std::log2(max_val))/4.0f;
	static_assert(max_val*std::exp2(0.0f*rate) == max_val);
	static_assert(max_val*std::exp2(4.0f*rate) == 0.5f);

	inline constinit color_scheme current_color_scheme{
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
		.selection_color = rgba_pixel{
			0.5f,
			1.0f,
			0.0f,
			1.0f
		},
		.mouse_focus_color = rgba_pixel{
			1.0f,
			0.0f,
			0.5f,
			1.0f
		},
		.keyboard_focus_color = rgba_pixel{
			0.5f,
			0.0f,
			1.0f,
			1.0f
		},
		.misc_colors{}
	};
}

#endif