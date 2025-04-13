//@	{"dependencies_extra":[{"ref":"./rowmajor_table.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_ROWMAJOR_TABLE_HPP
#define TERRAFORMER_UI_LAYOUTS_ROWMAJOR_TABLE_HPP

#include "./table.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include "ui/main/widget_collection_ref.hpp"

#include <memory>

namespace terraformer::ui::layouts
{
	class rowmajor_table:public table
	{
	public:
		static scaling set_default_cell_sizes(
			common_params const& params,
			state& state,
			main::widget_collection_view const& widgets
		);

		static scaling adjust_cell_sizes(
			struct params const& params,
			state& state,
			scaling available_size
		);

		static void get_widget_sizes_into(
			struct params2 const& params,
			state& state,
			scaling available_size
		);

		static void update_widget_locations(
			common_params const&,
			state const& state,
			main::widget_collection_ref& widgets
		);

		static constexpr table::algorithm algorithm{
			.set_default_cell_sizes = set_default_cell_sizes,
			.adjust_cell_sizes = adjust_cell_sizes,
			.update_widget_locations = update_widget_locations
		};

		using table::update_widget_locations;
		using table::set_default_cell_sizes;

		explicit rowmajor_table(size_t fixdim_cellcount):
			table{fixdim_cellcount, algorithm}
		{}
	};
}

#endif
