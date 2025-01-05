//@	{"target":{"name":"rowmajor_table.o"}}

#include "./rowmajor_table.hpp"

terraformer::scaling
terraformer::ui::layouts::rowmajor_table::update_widget_locations(
	main::widget_collection_ref& widgets
) const
{
	auto const widget_geometries = widgets.widget_geometries();
	auto x_offset = no_outer_margin?0.0f : margin_x;
	auto y_offset = no_outer_margin?0.0f : margin_y;

	auto const* const cols = m_fixdim_cellsizes.get();
	size_t col = 0;
	auto const colcount = m_fixdim_cellcount;
	auto row = m_dyndim_cellsizes.element_indices().front();
	auto const rowcount = std::size(m_dyndim_cellsizes);

	for(auto k : widgets.element_indices())
	{
		widget_geometries[k].where = location{
			x_offset,
			-y_offset,
			0.0f,
		};
		widget_geometries[k].origin = location{-1.0f, 1.0f, 0.0f};

		x_offset += cols[col];
		++col;
		if(col == colcount)
		{
			y_offset += m_dyndim_cellsizes[row];
			++row;
			if(row == rowcount)
			{ break; }
			col = 0;
			x_offset = no_outer_margin? 0.0f : margin_x;
		}
	}

	return scaling{
		m_width,
		m_height,
		1.0f,
	};
}

void terraformer::ui::layouts::rowmajor_table::minimize_cell_sizes(
	main::widget_collection_ref const& widgets
)
{
	auto const sizes = widgets.sizes();
	auto cols = m_fixdim_cellsizes.get();
	auto const colcount = m_fixdim_cellcount;
	std::fill_n(cols, colcount, 0.0f);
	m_dyndim_cellsizes.clear();
	auto max_height = 0.0f;
	size_t col = 0;
	auto const indices = widgets.element_indices();
	for(auto k : indices)
	{
		// NOTE: It is assumed that size is 0 if widget is collapsed
		cols[col] = std::max(sizes[k][0] + margin_x, cols[col]);
		max_height = std::max(
			sizes[k][1] + ((no_outer_margin && k == indices.back())?0.0f:margin_y),
			max_height
		);

		++col;
		if(col == colcount)
		{
			col = 0;
			m_dyndim_cellsizes.push_back(max_height);
			max_height = 0.0f;
		}
	}
	if(col != 0)
	{ m_dyndim_cellsizes.push_back(max_height); }

	m_height = std::accumulate(
		std::begin(m_dyndim_cellsizes),
		std::end(m_dyndim_cellsizes),
		no_outer_margin? 0.0f : margin_y
	);
	m_width = std::accumulate(cols, cols + colcount, margin_x)
		- (no_outer_margin? 2.0f*margin_x : 0.0f);
}
