//@	{"dependencies_extra":[{"ref":"./columnmajor_table.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_COLUMNMAJOR_TABLE_HPP
#define TERRAFORMER_UI_LAYOUTS_COLUMNMAJOR_TABLE_HPP

#include "./table.hpp"
#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include "ui/main/widget_collection_ref.hpp"

#include <memory>

namespace terraformer::ui::layouts
{
	class columnmajor_table:public table
	{
		static scaling update_widget_locations(
			common_params const&,
			state const& state,
			main::widget_collection_ref& widgets
		);

		static void minimize_cell_sizes(
			common_params const& params,
			state& state,
			main::widget_collection_ref const& widgets
		);

		static constexpr table::algorithm algorithm{
			.update_widget_locations = update_widget_locations,
			.minimize_cell_sizes = minimize_cell_sizes
		};

		using table::update_widget_locations;
		using table::minimize_cell_sizes;

		explicit columnmajor_table(size_t fixdim_cellcount):
			table{fixdim_cellcount, algorithm}
		{}
	};
}

#endif