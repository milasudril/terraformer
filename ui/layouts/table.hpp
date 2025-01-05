#ifndef TERRAFORMER_UI_LAYOUTS_TABLE_HPP
#define TERRAFORMER_UI_LAYOUTS_TABLE_HPP

#include "./common_params.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include "ui/main/widget_collection_ref.hpp"

#include <memory>

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
			float m_width;
			float m_height;
		};

		struct algorithm
		{
			scaling (*update_widget_locations)(common_params const&, state const&, main::widget_collection_ref&);
			void (*minimize_cell_sizes)(common_params const&, state&, main::widget_collection_ref const&);
		};

		explicit table(size_t fixdim_cellcount, algorithm algo):
			m_algo{algo},
			m_state{
				.m_fixdim_cellcount = fixdim_cellcount,
				.m_fixdim_cellsizes = std::make_unique<float[]>(fixdim_cellcount),
				.m_dyndim_cellsizes = single_array<float>{},
				.m_width = 0.0f,
				.m_height = 0.0f
			}
		{}

		scaling update_widget_locations(main::widget_collection_ref& widgets) const
		{ return m_algo.update_widget_locations(params, m_state, widgets); }

		void minimize_cell_sizes(main::widget_collection_ref const& widgets)
		{ return m_algo.minimize_cell_sizes(params, m_state, widgets); }

		common_params params;

	private:
		algorithm m_algo;
	protected:
		state m_state;
	};
}

#endif
