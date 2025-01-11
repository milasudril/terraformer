#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"
#include "ui/value_maps/asinh_value_map.hpp"
#include "lib/pixel_store/intensity.hpp"

#include <pretty/base.hpp>
#include <pretty/plot.hpp>
#include <array>


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

void dump_color_array(std::span<std::pair<terraformer::rgba_pixel, float> const> colors)
{
	puts("<table>");
	for(size_t k = std::size(colors); k != 0; --k)
	{
		puts("<tr>");
		make_colored_cell(make_html_color(colors[k - 1].first), k);
		printf("<td>%.2g</td>", colors[k - 1].second);
		puts("</tr>");
	}
	puts("</table>");
}

int main()
{
	using perceptual_color_intensity = terraformer::intensity<0.5673828125f, 1.0f, 0.060546875f>;
	static constexpr std::array<float, 4> x_vals{0.0f, 0.5f, 11.0f/12.0f, 12.0f/12.0f};
	static constexpr std::array<terraformer::rgba_pixel, 4> y_vals{
		normalize(terraformer::rgba_pixel{0.0f, 0.0f, 1.0f, 1.0f}, perceptual_color_intensity(0.5f)),
		normalize(terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 1.0f}, perceptual_color_intensity(0.5f + 1.0f/6.0f)),
		normalize(terraformer::rgba_pixel{1.0f, 0.0f, 0.0f, 1.0f}, perceptual_color_intensity(0.5f + 2.0f/6.0f)),
		normalize(terraformer::rgba_pixel{0.3333f, 0.3333f, 0.3333f, 1.0f}, perceptual_color_intensity(0.5f + 3.0f/6.0f)),
	};

	static constexpr terraformer::multi_span<float const, terraformer::rgba_pixel const> span{
		std::data(x_vals),
		std::data(y_vals),
		std::size(x_vals)
	};

	terraformer::linear_interpolation_table lut{span};

	terraformer::ui::value_maps::asinh_value_map value_map{
		266.3185546307779f,
		0.7086205026374324f*6.0f
	};

	std::array<std::pair<terraformer::rgba_pixel, float>,25> interp_vals{};
	for(size_t k = 0; k != std::size(interp_vals); ++k)
	{
		auto const xi = static_cast<float>(k)/static_cast<float>(std::size(interp_vals) - 1);
		interp_vals[k].first = lut(xi);
		interp_vals[k].second = value_map.to_value(xi);
	}
	dump_color_array(interp_vals);
}