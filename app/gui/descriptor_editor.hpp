#ifndef TERRAFORMER_APP_DESCRIPTOR_EDITOR_HPP
#define TERRAFORMER_APP_DESCRIPTOR_EDITOR_HPP

#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "ui/main/widget.hpp"
#include "ui/widgets/form.hpp"
#include "ui/widgets/table.hpp"

namespace terraformer::app
{
	template<class Impl>
	class descriptor_editor:public Impl
	{
	public:
		using Impl::Impl;

		struct table_descriptor
		{
			using input_widget_type = descriptor_editor<ui::widgets::table>;
			std::u8string_view label;
		};

		descriptor_editor_ref create_table(
			std::u8string_view label,
			descriptor_editor_ref::widget_orientation orientation,
			std::initializer_list<char8_t const*> field_names
		)
		{
			return descriptor_editor_ref{
				Impl::create_widget(
					table_descriptor{
						.label = label
					},
					orientation == descriptor_editor_ref::widget_orientation::horizontal?
						ui::main::widget_orientation::horizontal :
						ui::main::widget_orientation::vertical,
					field_names
				)
			};
		}
	};
}

#endif
