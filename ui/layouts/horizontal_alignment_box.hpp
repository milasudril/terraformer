//@	{"dependencies_extra":[{"ref":"./horizontal_alignment_box.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_HORIZONTAL_ALIGNMENT_BOX_HPP
#define TERRAFORMER_UI_LAYOUTS_HORIZONTAL_ALIGNMENT_BOX_HPP

#include "./common_params.hpp"

#include "ui/main/widget_collection_ref.hpp"

namespace terraformer::ui::layouts
{
	class horizontal_alignment_box
	{
	public:
		scaling update_widget_locations(main::widget_collection_ref& widgets) const;

		void minimize_cell_sizes(main::widget_collection_ref const& widgets);

		common_params params;

	private:
		float m_width{0.0f};
		float m_height{0.0f};
	};
}

#endif
