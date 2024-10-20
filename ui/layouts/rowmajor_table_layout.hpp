//@	{"dependencies_extra":[{"ref":"./rowmajor_table_layout.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_ROWMAJOR_TABLE_LAYOUT_HPP
#define TERRAFORMER_UI_WIDGETS_ROWMAJOR_TABLE_LAYOUT_HPP

#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include "ui/main/widget_collection_ref.hpp"

#include <memory>

namespace terraformer::ui::widgets
{
	class rowmajor_table_layout
	{
	public:
		explicit rowmajor_table_layout(size_t colcount):
			m_colcount{colcount},
			m_colwidths{std::make_unique<float[]>(colcount)}
		{}
			
		scaling update_widget_locations(main::widget_collection_ref& widgets) const;
		
		void minimize_cell_sizes(main::widget_collection_ref const& widgets);

		float margin_x;
		
		float margin_y;

	private:
		size_t m_colcount;
		std::unique_ptr<float[]> m_colwidths;
		single_array<float> m_rowheights;
		float m_width;
		float m_height;
	};
}

#endif
