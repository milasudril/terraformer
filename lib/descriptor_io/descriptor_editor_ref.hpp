#ifndef TERRAFORMER_DESCRIPTOR_EDITOR_REF_HPP
#define TERRAFORMER_DESCRIPTOR_EDITOR_REF_HPP

#include <initializer_list>
#include <string_view>

namespace terraformer
{
	class descriptor_editor_ref
	{
	public:
		enum class widget_orientation{horizontal, vertical};

		descriptor_editor_ref create_table(
			std::u8string_view,
			widget_orientation,
			std::initializer_list<char8_t const*>
		)
		{ return *this; }
	};
}

#endif