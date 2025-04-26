//@	{"target": {"name":"table_new.o"}}

#include "./table_new.hpp"

namespace
{
	size_t ceil_div(size_t x, size_t y)
	{ return x / y + (x % y == 0? 0 : 1); }
}

void terraformer::ui::layouts::table_new::set_cell_sizes_to(std::span<scaling const> sizes_in)
{
	if(std::size(sizes_in) == 0)
	{ return; }

	switch(m_cell_order)
	{
		case cell_order::row_major:
		{
			m_rows = cell_size_array{row_count{ceil_div(std::size(sizes_in), std::size(m_cols))}};
			break;
		}

		case cell_order::column_major:
		{
			m_cols = cell_size_array{column_count{ceil_div(std::size(sizes_in), std::size(m_rows))}};
			break;
		}
	}
}