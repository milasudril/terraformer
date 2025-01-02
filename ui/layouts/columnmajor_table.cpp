//@	{"target":{"name":"columnmajor_table.o"}}

#include "./columnmajor_table.hpp"

terraformer::scaling
terraformer::ui::layouts::columnmajor_table::update_widget_locations(
	main::widget_collection_ref& widgets
) const
{
	auto const widget_geometries = widgets.widget_geometries();
	auto x_offset = no_outer_margin? 0.0f : margin_x;
	auto y_offset = no_outer_margin? 0.0f : margin_y;

	auto const* const rows = m_rowheights.get();
	size_t row = 0;
	auto const rowcount = m_rowcount;
	auto col = m_colwidths.element_indices().front();
	auto const colcount = std::size(m_colwidths);

	for(auto k : widgets.element_indices())
	{
		widget_geometries[k].where = location{
			x_offset,
			-y_offset,
			0.0f,
		};
		widget_geometries[k].origin = location{-1.0f, 1.0f, 0.0f};

		y_offset += rows[row];
		++row;
		if(row == rowcount)
		{
			x_offset += m_colwidths[col];
			++col;
			if(col == colcount)
			{ break; }
			row = 0;
			y_offset = no_outer_margin? 0.0f : margin_y;
		}
	}

	return scaling{
		m_width,
		m_height,
		1.0f,
	};
}

void terraformer::ui::layouts::columnmajor_table::minimize_cell_sizes(
	main::widget_collection_ref const& widgets
)
{
	auto const sizes = widgets.sizes();
	auto rows = m_rowheights.get();
	auto const rowcount = m_rowcount;
	std::fill_n(rows, rowcount, 0.0f);
	m_colwidths.clear();
	auto max_width = 0.0f;
	size_t row = 0;
	auto const indices = widgets.element_indices();
	for(auto k : indices)
	{
		// NOTE: It is assumed that size is 0 if widget is collapsed
		rows[row] = std::max(sizes[k][1] + margin_y, rows[row]);
		max_width = std::max(
			sizes[k][0] +	((no_outer_margin && k == indices.back())? 0.0f : margin_x),
			max_width
		);

		++row;
		if(row == rowcount)
		{
			row = 0;
			m_colwidths.push_back(max_width);
			max_width = 0.0f;
		}
	}
	if(row != 0)
	{ m_colwidths.push_back(max_width); }

	m_width = std::accumulate(
		std::begin(m_colwidths),
		std::end(m_colwidths),
		no_outer_margin?
			0.0f:margin_x
	);
	m_height = std::accumulate(rows, rows + rowcount, margin_y)
		 - (no_outer_margin? 2.0f*margin_y : 0.0f);
}
