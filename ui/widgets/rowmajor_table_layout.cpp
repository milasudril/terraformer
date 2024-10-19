//@	{"target":{"name":"rowmajor_table_layout.o"}}

#include "./rowmajor_table_layout.hpp"

terraformer::scaling
terraformer::ui::widgets::rowmajor_table_layout::	update_widget_locations(
	main::widget_collection_ref& widgets
) const
{
	auto const sizes = std::as_const(widgets).sizes();
	auto const widget_geometries = widgets.widget_geometries();
	auto const widget_states = widgets.widget_states();

	auto const n = std::size(widgets);
	auto min_width = 0.0f;
	auto max_width = std::numeric_limits<float>::infinity();
	auto height = margin_y;

	for(auto k = widgets.first_element_index(); k != n; ++k)
	{
		if(!widget_states[k].collapsed) [[likely]]
		{
			auto const& size = sizes[k];
			widget_geometries[k].where = location{
				margin_x,
				-height,
				0.0f
			};
			widget_geometries[k].origin = terraformer::location{-1.0f, 1.0f, 0.0f};
			widget_geometries[k].size = size;
			min_width = std::max(min_width, size[0]);
			max_width = std::min(max_width, size[0]);
			height += widget_geometries[k].size[1] + margin_y;
		}
	}

	return scaling{
		min_width + 2.0f*margin_x,
		height,
		1.0f
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
