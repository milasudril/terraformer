#ifndef TERRAFORMER_UI_LAYOUTS_TABLE_HPP
#define TERRAFORMER_UI_LAYOUTS_TABLE_HPP

#include "./common_params.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include "lib/math_utils/ratio.hpp"
#include "ui/main/widget_collection_ref.hpp"

#include <memory>
#include <variant>

namespace terraformer::ui::layouts
{
	class table
	{
	public:
		struct state
		{
			size_t m_fixdim_cellcount;
			std::unique_ptr<float[]> m_fixdim_cellsizes;
			single_array<float> m_dyndim_cellsizes;
		};


		template<auto DimensionTag>
		struct cell_size
		{
			struct minimize{};
			struct expand{};
			std::variant<minimize, expand, ratio, float> value;
		};

		using column_width = cell_size<0>;
		using row_height = cell_size<1>;

		struct params:common_params
		{
			single_array<column_width> column_widths;
			single_array<row_height> row_heights;
		};

		struct algorithm
		{
			scaling (*minimize_cell_sizes)(common_params const&, state&, main::widget_collection_ref const&);
			scaling (*adjust_cell_sizes)(struct params const&, state&, scaling available_size);
			void (*update_widget_locations)(common_params const&, state const&, main::widget_collection_ref&);
		};

		explicit table(size_t fixdim_cellcount, algorithm algo):
			m_algo{algo},
			m_state{
				.m_fixdim_cellcount = fixdim_cellcount,
				.m_fixdim_cellsizes = std::make_unique<float[]>(fixdim_cellcount),
				.m_dyndim_cellsizes = single_array<float>{}
			}
		{}

		scaling minimize_cell_sizes(main::widget_collection_ref const& widgets)
		{ return m_algo.minimize_cell_sizes(params, m_state, widgets); }

		scaling adjust_cell_sizes(scaling available_size)
		{ return m_algo.adjust_cell_sizes(params, m_state, available_size); }

		void update_widget_locations(main::widget_collection_ref& widgets) const
		{ m_algo.update_widget_locations(params, m_state, widgets); }

		struct params params;

	private:
		algorithm m_algo;
	protected:
		state m_state;
	};
}

#endif
