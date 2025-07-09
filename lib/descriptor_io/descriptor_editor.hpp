#ifndef TERRAFORMER_DESCRIPTOR_EDITOR_HPP
#define TERRAFORMER_DESCRIPTOR_EDITOR_HPP

#include "ui/main/widget.hpp"

namespace terraformer
{
	class descriptor_editor
	{
	public:
		enum class widget_orientation{horizontal, vertical};

		descriptor_editor& create_table(
			std::u8string_view,
			widget_orientation,
			std::initializer_list<char8_t const*>
		)
		{ return *this; }
	};
}

#endif