#ifndef TERRAFORMER_APP_DESCRIPTOR_EDITOR_HPP
#define TERRAFORMER_APP_DESCRIPTOR_EDITOR_HPP

#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "ui/widgets/form.hpp"

namespace terraformer::app
{
	class descriptor_editor:public ui::widgets::form
	{
	public:
		using form::form;

		template<class ... T>
		descriptor_editor_ref create_table(T&&...)
		{
			return descriptor_editor_ref{*this};
		}
	};
}

#endif
