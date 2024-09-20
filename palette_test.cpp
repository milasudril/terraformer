#include "ui/theming/fixed_intensity_colormap.hpp"

#include <cassert>

using palette = std::array<terraformer::rgba_pixel, 12>;

template<class Colormap>
palette generate_palette(Colormap const& cmap)
{
	palette ret{};
	for(size_t k = 0; k != std::size(ret); ++k)
	{
		auto const t = static_cast<float>(k)/std::size(ret);
		ret[k] = cmap(t);
	}
	
	return ret;
}

palette brighten(palette const& colors, float target_intensity)
{
	palette ret{};
	for(size_t k = 0; k != std::size(ret); ++k)
	{
		ret[k] = terraformer::ui::theming::fixed_intensity_colormap_helpers::brighten(colors[k], target_intensity);
	}
	return ret;
}

palette darken(palette const& colors, float target_intensity)
{
	palette ret{};
	for(size_t k = 0; k != std::size(ret); ++k)
	{
		ret[k] = terraformer::ui::theming::fixed_intensity_colormap_helpers::normalize_blend_white(colors[k], target_intensity);
	}
	return ret;
}

struct html_color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

html_color make_html_color(terraformer::rgba_pixel input)
{
	return html_color{
		.r = static_cast<uint8_t>(255.0f*std::pow(std::min(input.red(), 1.0f), 1.0f/2.2f)),
		.g = static_cast<uint8_t>(255.0f*std::pow(std::min(input.green(), 1.0f), 1.0f/2.2f)),
		.b = static_cast<uint8_t>(255.0f*std::pow(std::min(input.blue(), 1.0f), 1.0f/2.2f))
	};
}

void make_colored_cell(html_color rgb, size_t index, size_t colspan = 1)
{
	printf("<td colspan=\"%zu\" style=\"background-color: #%02x%02x%02x; text-align:center; width: 4ex; height:4ex; padding:0ex; vertical-align:middle\">"
		"%zu"
		"</td>",
		colspan,
		 rgb.r,
		 rgb.g,
		 rgb.b,
		 index
	);
}

void dump_color_array(std::span<terraformer::rgba_pixel const> colors)
{
	puts("<table>");
	puts("<tr>");
	for(size_t k = 0; k != std::size(colors); ++k)
	{
		make_colored_cell(make_html_color(colors[k]), k);
	
	}
	puts("</tr>");
	puts("</table>");
}

void dump_four_color_combination(std::span<terraformer::rgba_pixel const> colors, size_t offset)
{
	auto const n = std::size(colors);
	assert(n%4 == 0);
	auto const step = n/4;
	puts("<table>");
	puts("<tr>");
	make_colored_cell(make_html_color(colors[(0 + offset)%n]), (0 + offset)%n);
	make_colored_cell(make_html_color(colors[(1*step + offset)%n]), (1*step + offset)%n);
	puts("</tr><tr>");
	make_colored_cell(make_html_color(colors[(2*step + offset)%n]), (2*step + offset)%n);
	make_colored_cell(make_html_color(colors[(3*step + offset)%n]), (3*step + offset)%n);
	puts("</tr>");
	puts("</table>");

	puts("<table>");
	puts("<tr>");
	make_colored_cell(make_html_color(colors[(0 + offset)%n]), (0 + offset)%n);
	make_colored_cell(make_html_color(colors[(1*step + offset)%n]), (1*step + offset)%n);
	puts("</tr><tr>");
	make_colored_cell(make_html_color(colors[(3*step + offset)%n]), (3*step + offset)%n);
	make_colored_cell(make_html_color(colors[(2*step + offset)%n]), (2*step + offset)%n);
	puts("</tr>");
	puts("</table>");
}

void dump_all_four_color_combos(std::span<terraformer::rgba_pixel const> colors)
{
	puts("<table>");
	puts("<tr>");
	for(size_t k = 0; k!=std::size(colors)/4; ++k)
	{
		puts("<td>");
		dump_four_color_combination(colors, k);
		puts("</td>");
	}
	puts("</tr>");
	puts("</table>");
}

void dump_three_color_combination(std::span<terraformer::rgba_pixel const> colors, size_t offset)
{
	auto const n = std::size(colors);
	assert(n == 12);
	puts("<table>");
	puts("<tr>");
	make_colored_cell(make_html_color(colors[(0 + offset)%n]), (0 + offset)%n, 2);
	puts("</tr><tr>");
	make_colored_cell(make_html_color(colors[(5 + offset)%n]), (5 + offset)%n);
	make_colored_cell(make_html_color(colors[(7 + offset)%n]), (7 + offset)%n);
	puts("</tr>");
	puts("</table>");
}

void print_header(size_t level, char const* name)
{
	printf("<h%zu>%s</h%zu>\n", level, name, level);
}


void dump_all_three_color_combos(std::span<terraformer::rgba_pixel const> colors)
{
	print_header(3, "\"Warm\" primary color");
	puts("<table>");
	puts("<tr>");
	for(size_t k = 0; k!=std::size(colors)/2; ++k)
	{
		puts("<td>");
		dump_three_color_combination(colors, k);
		puts("</td>");
	}
	puts("</tr>");
	puts("</table>");

	print_header(3, "\"Cold\" primary color");
	puts("<table>");
	puts("<tr>");
	for(size_t k = std::size(colors)/2; k!=std::size(colors); ++k)
	{
		puts("<td>");
		dump_three_color_combination(colors, k);
		puts("</td>");
	}
	puts("</tr>");
	puts("</table>");
}

template<class Colormap>
void test_palette(Colormap const& cmap)
{
	auto const colors = generate_palette(cmap);
	auto const dark_colors = darken(colors, 0.25);
	auto const bright_colors = brighten(colors, 1.0f);

	print_header(2, "List of all colors");
	dump_color_array(bright_colors);
	dump_color_array(colors);
	dump_color_array(dark_colors);
	puts("<p>These colors should appear</p>");
	puts("<ul>");
	puts("<li>Equally bright</li>");
	puts("<li>Equally saturated</li>");
	puts("<li>Equally spaced in hue space</li>");
	puts("</ul>");

	print_header(2, "List of all 4-color combination");
	dump_all_four_color_combos(colors);
	puts("<p>These groups of four colors should not favor any particular index</p>");

	print_header(2, "List of all 3-color combination");
	puts("<p>The primary color should work together with the secondary colors, "
		"and the secondary colors should work together.</p>");
	dump_all_three_color_combos(colors);
}


int main()
{
	print_header(1, "Testing color palette");
	test_palette(terraformer::ui::theming::fixed_intensity_colormap{});
}