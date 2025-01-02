//@	{"dependencies_extra":[{"ref":"./columnmajor_table.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_COLUMNMAJOR_TABLE_HPP
#define TERRAFORMER_UI_LAYOUTS_COLUMNMAJOR_TABLE_HPP

#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include "ui/main/widget_collection_ref.hpp"

#include <memory>

namespace terraformer::ui::layouts
{
	class columnmajor_table
	{
	public:
		explicit columnmajor_table(size_t rowcount):
			m_rowcount{rowcount},
			m_rowheights{std::make_unique<float[]>(rowcount)}
		{}

		scaling update_widget_locations(main::widget_collection_ref& widgets) const;

		void minimize_cell_sizes(main::widget_collection_ref const& widgets);

		float margin_x = 2.0f;
		float margin_y = 2.0f;
		bool no_outer_margin = false;

	private:
		size_t m_rowcount;
		std::unique_ptr<float[]> m_rowheights;
		single_array<float> m_colwidths;
		float m_width;
		float m_height;
	};
}

#endif