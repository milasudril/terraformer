//@	{"dependencies_extra":[{"ref":"./size_only_layout.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_SIZE_ONLY_LAYOUT_HPP
#define TERRAFORMER_UI_LAYOUTS_SIZE_ONLY_LAYOUT_HPP

#include "lib/common/spaces.hpp"
#include "ui/main/widget_collection_ref.hpp"

#include <memory>

namespace terraformer::ui::layouts
{
	class size_only_layout
	{
	public:
		scaling update_widget_locations(main::widget_collection_ref const& widgets) const;

		void minimize_cell_sizes(main::widget_collection_ref const&)
		{ }
	};
}

#endif
