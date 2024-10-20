//@	{"target":{"name":"rowmajor_table_layout.o"}}

#include "./rowmajor_table_layout.hpp"

terraformer::scaling
terraformer::ui::layouts::rowmajor_table_layout::update_widget_locations(
	main::widget_collection_ref& widgets
) const
{	
	auto const sizes = widgets.sizes();
	auto const widget_geometries = widgets.widget_geometries();
	auto x_offset = margin_x;
	auto y_offset = margin_y;

	auto const n = std::size(widgets);
	auto const* const cols = m_colwidths.get();
	size_t col = 0;
	auto const colcount = m_colcount;
	auto row = m_rowheights.first_element_index();
	auto const rowcount = std::size(m_rowheights);
	
	for(auto k = widgets.first_element_index(); k != n; ++k)
	{
		widget_geometries[k].where = location{
			x_offset,
			-y_offset,
			0.0f,
		};
		widget_geometries[k].size = sizes[k];
		widget_geometries[k].origin = location{-1.0f, 1.0f, 0.0f};

		x_offset += cols[col];
		++col;
		if(col == colcount)
		{
			y_offset += m_rowheights[row];
			++row;
			if(row == rowcount)
			{ break; }
			col = 0;
			x_offset = margin_x;
		}
	}
	
	return scaling{
		m_width,
		m_height,
		1.0f,
	};
}

void terraformer::ui::layouts::rowmajor_table_layout::minimize_cell_sizes(
	main::widget_collection_ref const& widgets
)
{
	auto const sizes = widgets.sizes();
	auto const n = std::size(widgets);
	auto cols = m_colwidths.get();
	auto const colcount = m_colcount;
	std::fill_n(cols, colcount, 0.0f); 
	m_rowheights.clear();
	auto max_height = 0.0f;
	size_t col = 0;
	for(auto k = widgets.first_element_index(); k != n; ++k)
	{
		// NOTE: It is assumed that size is 0 if widget is collapsed
		cols[col] = std::max(sizes[k][0] + margin_x, cols[col]);
		max_height = std::max(sizes[k][1] + margin_y, max_height);
		
		++col;
		if(col == colcount)
		{
			col = 0;
			m_rowheights.push_back(max_height);
			max_height = 0.0f;
		}
	}
	if(col != 0 && n != widgets.first_element_index())
	{ m_rowheights.push_back(max_height); }
	
	m_height = std::accumulate(std::begin(m_rowheights), std::end(m_rowheights), margin_y);
	m_width = std::accumulate(cols, cols + colcount, margin_x);
}
