//@	{"dependencies_extra":[{"ref":"./rowmajor_table.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_ROWMAJOR_TABLE_HPP
#define TERRAFORMER_UI_LAYOUTS_ROWMAJOR_TABLE_HPP

#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include "ui/main/widget_collection_ref.hpp"

#include <memory>

namespace terraformer::ui::layouts
{
	class rowmajor_table
	{
	public:
		explicit rowmajor_table(size_t colcount):
			m_colcount{colcount},
			m_colwidths{std::make_unique<float[]>(colcount)}
		{}

		scaling update_widget_locations(main::widget_collection_ref& widgets) const;

		void minimize_cell_sizes(main::widget_collection_ref const& widgets);

		float margin_x = 2.0f;
		float margin_y = 2.0f;
		bool no_outer_margin = false;

	private:
		size_t m_colcount;
		std::unique_ptr<float[]> m_colwidths;
		single_array<float> m_rowheights;
		float m_width;
		float m_height;
	};
}

#endif
