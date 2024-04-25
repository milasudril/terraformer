#ifndef TERRAFORMER_UI_THEMING_CURSORS_HPP
#define TERRAFORMER_UI_THEMING_CURSORS_HPP

#include "./stock_cursors.hpp"

namespace terraformer::ui::theming
{
	struct cursor_set
	{
		// TODO: May need to allow ownership within a cursor_set
		cursor_view main;
	};

	constinit cursor_set current_cursor_set{
		.main = make_cursor_view(stock_cursors::main)
	};
}

#endif