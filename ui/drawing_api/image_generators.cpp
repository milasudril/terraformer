//@	{"target":{"name":"image_generators.o"}}

#include "./image_generators.hpp"

namespace
{
	struct beveled_rectangle_corner_piece
	{
		int32_t size;
	};


	bool inside(int32_t x, int32_t y, beveled_rectangle_corner_piece piece)
	{
		return (x>= 0 && y >= 0) && (x < piece.size && y < piece.size) && x < y;
	}
}

terraformer::image terraformer::ui::drawing_api::generate(beveled_rectangle const& params)
{

	auto const w = params.width;
	auto const h = params.height;
	image ret{params.domain_size.width, params.domain_size.height};
	auto const border_thickness = params.border_thickness;
	auto const upper_left_color = params.upper_left_color;
	auto const lower_right_color = params.lower_right_color;
	auto const fill_color = params.fill_color;
	auto const origin_x = params.origin_x;
	auto const origin_y = params.origin_y;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const write_x = x + origin_x;
			auto const write_y = y + origin_y;

			if(write_x < ret.width() && write_y < ret.height())
			{
				auto const in_lower_left = inside(
					static_cast<int32_t>(x),
					static_cast<int32_t>(h) - static_cast<int32_t>(y) - 1,
					beveled_rectangle_corner_piece{static_cast<int32_t>(border_thickness)}
				);

				auto const in_upper_right = inside(
					static_cast<int32_t>(x) - static_cast<int32_t>(w) + static_cast<int32_t>(border_thickness),
					static_cast<int32_t>(border_thickness) - static_cast<int32_t>(y) - 1,
					beveled_rectangle_corner_piece{static_cast<int32_t>(border_thickness)}
				);

				auto const in_left_rect = x < border_thickness && y < h - border_thickness;
				auto const in_top_rect = y < border_thickness && x < w - border_thickness;

				auto const border = in_lower_left || in_upper_right || in_left_rect || in_top_rect?
					upper_left_color:
					lower_right_color;

				ret(write_x, write_y) = (x>=border_thickness && x <= w - (border_thickness + 1)) && (y >= border_thickness && y <= h - (border_thickness + 1)) ? fill_color : border;
			}
		}
	}
	return ret;
}

terraformer::image terraformer::ui::drawing_api::generate(flat_rectangle const& params)
{
	auto const w = params.width;
	auto const h = params.height;
	image ret{params.domain_size.width, params.domain_size.height};
	auto const border_thickness = params.border_thickness;
	auto const border_color = params.border_color;
	auto const fill_color = params.fill_color;
	auto const origin_x = params.origin_x;
	auto const origin_y = params.origin_y;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const write_x = x + origin_x;
			auto const write_y = y + origin_y;

			if(write_x < ret.width() && write_y < ret.height())
			{
				ret(write_x, write_y) =
					(x>=border_thickness && x <= w - (border_thickness + 1)) &&
						(y >= border_thickness && y <= h - (border_thickness + 1)) ?
						fill_color :
						border_color;
			}
		}
	}
	return ret;
}

terraformer::image terraformer::ui::drawing_api::generate(beveled_disc const& params)
{
	auto const r = params.radius;
	image ret{params.domain_size.width, params.domain_size.height};
	auto const border_thickness = params.border_thickness;
	auto const inner_radius = static_cast<int64_t>(r) - static_cast<int64_t>(border_thickness);
	auto const upper_left_color = params.upper_left_color;
	auto const lower_right_color = params.lower_right_color;
	auto const fill_color = params.fill_color;
	auto const origin_x = params.origin_x;
	auto const origin_y = params.origin_y;

	for(uint32_t y = 0; y != 2*r; ++y)
	{
		for(uint32_t x = 0; x != 2*r; ++x)
		{
			auto const write_x = x + origin_x;
			auto const write_y = y + origin_y;

			if(write_x < ret.width() && write_y < ret.height())
			{
				auto const dx = static_cast<int64_t>(x) - static_cast<int64_t>(r);
				auto const dy = static_cast<int64_t>(y) - static_cast<int64_t>(r);
				auto const d2 = dx*dx + dy*dy;
				if(d2 <= inner_radius*inner_radius)
				{ ret(write_x, write_y) = fill_color; }
				else
				if(d2 <= r*r)
				{
					auto const theta = std::atan2(static_cast<float>(dy), static_cast<float>(dx))
						+ std::numbers::pi_v<float>/4.0f;
					if(theta >= 0.0f && theta <= std::numbers::pi_v<float>)
					{ ret(write_x, write_y) = lower_right_color; }
					else
					{ ret(write_x, write_y) = upper_left_color; }
				}
				else
				{ ret(write_x, write_y) = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}; }
			}
		}
	}
	return ret;
}

terraformer::image terraformer::ui::drawing_api::convert_mask(span_2d<uint8_t const> input)
{
	auto const w = input.width();
	auto const h = input.height();
	image ret{w, h};
	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const mask_val = (x < w && y < h)?
				static_cast<float>(input(x, y))/255.0f:
				0.0f;
			ret(x, y) = rgba_pixel{mask_val, mask_val, mask_val, mask_val};
		}
	}
	return ret;
}

terraformer::image terraformer::ui::drawing_api::transpose(span_2d<rgba_pixel const> input)
{
	image ret{input.height(), input.width()};
	auto const h = ret.height();
	auto const w = ret.width();
	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{ ret(x, y) = input(x, y); }
	}

	return ret;
}
