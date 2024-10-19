//@	{"target":{"name":"rowmajor_table_layout.o"}}

#include "./rowmajor_table_layout.hpp"

terraformer::scaling
terraformer::ui::widgets::rowmajor_table_layout::update_widget_locations(
	main::widget_collection_ref& widgets
) const
{
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
		x_offset += cols[col] + margin_x;
		y_offset += m_rowheights[row] + margin_y;

		++col;
		if(col == colcount)
		{
			col = 0;
			++row;
			if(row == rowcount)
			{ break; }
		}
	}
	return scaling{
		x_offset,
		y_offset,
		1.0f,
	};
}

void terraformer::ui::widgets::rowmajor_table_layout::minimize_cell_sizes(
	main::widget_collection_view const& widgets
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
		cols[col] = std::max(sizes[k][0], cols[col]);
		max_height = std::max(sizes[k][1], max_height);
		
		++col;
		if(col == colcount)
		{
			col = 0;
			m_rowheights.push_back(max_height);
			max_height = 0.0f;
		}
	}
}
