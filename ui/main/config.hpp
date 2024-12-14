#ifndef TERRAFORMER_UI_MAIN_CONFIG_HPP
#define TERRAFORMER_UI_MAIN_CONFIG_HPP

#include "./texture_types.hpp"

#include "lib/pixel_store/rgba_pixel.hpp"
#include "ui/font_handling/text_shaper.hpp"

#include <array>
#include <memory>

namespace terraformer::ui::main
{
	struct panel_colors
	{
		rgba_pixel background;
	};

	struct widget_colors
	{
		rgba_pixel background;
		rgba_pixel foreground;
		rgba_pixel selection;
	};

	struct twocolor_gradient
	{
		rgba_pixel begin;
		rgba_pixel end;
	};

	struct status_colors
	{
		twocolor_gradient progress_meter;
		twocolor_gradient resource_usage_meter;

		rgba_pixel error_indication;
		rgba_pixel warning_indication;
		rgba_pixel progress_indication;
		rgba_pixel ok_indication;
		rgba_pixel info_indication;
	};

	struct event_routing_colors
	{
		rgba_pixel cursor_color;
		rgba_pixel mouse_focus_color;
		rgba_pixel keyboard_focus_color;
	};

	struct palettes
	{
		std::array<rgba_pixel, 12> dark;
		std::array<rgba_pixel, 12> mid;
		std::array<rgba_pixel, 12> bright;
	};

	struct color_scheme
	{
		panel_colors main_panel;
		panel_colors other_panel;
		widget_colors input_area;
		widget_colors command_area;
		widget_colors output_area;

		status_colors status_indicator;

		event_routing_colors mouse_kbd_tracking;

		palettes misc_colors;
	};

	struct panel_look
	{
		panel_colors colors;
		immutable_shared_texture background_texture;
		float padding;
	};

	struct widget_look
	{
		widget_colors colors;
		float padding;
		float border_thickness;
		std::shared_ptr<font_handling::font const> font;
	};

	struct event_routing_look
	{
		event_routing_colors colors;
		float border_thickness;
	};

	struct stock_textures
	{
		immutable_shared_texture null;
		immutable_shared_texture black;
		immutable_shared_texture white;
		immutable_shared_texture test_pattern;
	};

	struct config
	{
		panel_look main_panel;
		panel_look other_panel;
		widget_look input_area;
		widget_look command_area;
		widget_look output_area;
		status_colors status_indicator;
		event_routing_look mouse_kbd_tracking;
		palettes misc_colors;
		stock_textures misc_textures;
	};
}
#endif