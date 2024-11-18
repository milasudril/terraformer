//@	{"target":{"name":"text_shaper.o"}}

#include "./text_shaper.hpp"

#include <algorithm>

terraformer::span_2d_extents
terraformer::ui::font_handling::compute_extents(shaping_result const& result)
{
	uint32_t width = 0;
	uint32_t height = 0;
	auto const n = result.glyph_count;
	auto const glyph_info = result.glyph_info;
	auto const glyph_pos = result.glyph_pos;
	for(size_t i = 0; i != n; ++i)
	{
		auto const& glyph = get_glyph(result, glyph_index{glyph_info[i].codepoint});
		width += (i != n - 1)?
			glyph_pos[i].x_advance :
			std::max(glyph.image.width()*64, static_cast<uint32_t>(glyph_pos[i].x_advance));
		height += (i != n - 1)? -glyph_pos[i].y_advance : glyph.image.height()*64;
	}

	auto const& renderer = result.renderer.get();

	height = std::max(static_cast<uint32_t>(renderer.get_global_glyph_height()), height);
	width = std::max(static_cast<uint32_t>(renderer.get_global_glyph_width()), width);

	return span_2d_extents{
		.width = static_cast<uint32_t>(std::ceil(static_cast<float>(width)/64.0f)),
		.height = static_cast<uint32_t>(std::ceil(static_cast<float>(height)/64.0f))
	};
}

terraformer::ui::font_handling::glyph_sequence::glyph_sequence(shaping_result const& result):
	m_extents{compute_extents(result)}
{
	// TODO: Fix vertical rendering

	m_content.resize(static_cast<size_type>(result.glyph_count));
	auto const glyph_info = result.glyph_info;
	auto const glyph_pos = result.glyph_pos;
	auto const ascender = result.renderer.get().get_ascender();
	uint64_t cursor_x = 0;
	uint64_t cursor_y = 0;

	auto const locs = m_content.get<0>();
	auto const indices = m_content.get<1>();
	auto const glyph_ptrs = m_content.get<2>();

	for(auto item : m_content.element_indices())
	{
		auto const i = item.get();
		auto const& glyph = get_glyph(result, glyph_index{glyph_info[i].codepoint});
		auto const x_offset  = -glyph_pos[i].x_offset;
		auto const y_offset  = glyph_pos[i].y_offset;

		locs[item] = location{
			static_cast<float>(cursor_x + x_offset)/64.0f + static_cast<float>(glyph.x_offset),
			static_cast<float>(cursor_y + y_offset + ascender)/64.0f - static_cast<float>(glyph.y_offset),
			0.0f
		};
		indices[item] = glyph_info[i].cluster;
		glyph_ptrs[item] = &glyph;

		cursor_x += glyph_pos[i].x_advance;
		cursor_y -= glyph_pos[i].y_advance;
	}
}

terraformer::basic_image<uint8_t>
terraformer::ui::font_handling::render(glyph_sequence const& seq)
{
	auto const size = seq.extents();
	terraformer::basic_image<uint8_t> ret{size.width, size.height};

	auto const locations = seq.locations();
	auto const glyph_ptrs = seq.glyph_pointers();

	for(auto item : seq.element_indices())
	{
		auto const& glyph = *glyph_ptrs[item];
		auto const loc = locations[item];

		printf("%.8g ", loc[0]);

		render(
			glyph,
			ret.pixels(),
			static_cast<uint32_t>(loc[0] + 0.5f),
			static_cast<uint32_t>(loc[1] + 0.5f)
		);
	}
	putchar('\n');

	return ret;
}

terraformer::ui::font_handling::glyph_index_range
terraformer::ui::font_handling::find_glyph_index_range(
	glyph_sequence const& seq,
	index_range<size_t> selection
)
{
	if(selection.empty())
	{ return glyph_index_range{}; }

	for(auto i : seq.input_indices())
	{ printf("%zu ", i); }
	putchar('\n');


	printf("Looking for glyhs in range %zu %zu\n", selection.front(), selection.back() + 1);
	auto const indices = seq.input_indices();
	auto const i_start = std::ranges::find_if(
		std::ranges::reverse_view{indices},
		[search_for = selection.front()](auto val){
			return val <= search_for;
		}
	).base() - 1;
	if(i_start == std::end(indices))
	{  return glyph_index_range{}; }

	auto i_end = std::find_if(
		i_start + 1,
		std::end(indices),
		[search_for = selection.back() + 1](auto val){
			return val >= search_for;
		}
	);
	return glyph_index_range{
		glyph_sequence::index_type{static_cast<size_t>(i_start - std::begin(indices))},
		i_end == std::end(indices) ?
			glyph_sequence::npos:
			glyph_sequence::index_type{static_cast<size_t>(i_end - std::begin(indices))},
	};
}
