#ifndef TERRAFORMER_UI_THEMING_CURSORS_HPP
#define TERRAFORMER_UI_THEMING_CURSORS_HPP

#include "./stock_cursors.hpp"

namespace terraformer::ui::theming
{
	struct cursor_set
	{
		// TODO: May need to allow ownership within a cursor_set
		cursor_view main;
		cursor_view horz_resize;
		cursor_view vert_resize;
		cursor_view resize;
	};

	constinit cursor_set current_cursor_set{
		.main = make_cursor_view(stock_cursors::main),
		.horz_resize = make_cursor_view(stock_cursors::horz_resize),
		.vert_resize = make_cursor_view(stock_cursors::vert_resize),
		.resize = make_cursor_view(stock_cursors::resize)
	};
}

#endif