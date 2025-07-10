#ifndef TERRAFORMER_DESCRIPTOR_EDITOR_REF_HPP
#define TERRAFORMER_DESCRIPTOR_EDITOR_REF_HPP

#include "lib/common/unique_resource.hpp"

#include <initializer_list>
#include <string_view>

namespace terraformer
{
	class descriptor_editor_ref
	{
	public:
		enum class widget_orientation{horizontal, vertical};

		template<class DescriptorEditor>
		explicit descriptor_editor_ref(DescriptorEditor& editor):
			m_handle{editor}
		{}

		descriptor_editor_ref create_table(
			std::u8string_view label,
			widget_orientation orientation,
			std::initializer_list<char8_t const*> field_names
		) const
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.create_table(pointer, label, orientation, field_names);
		}

	private:
		struct vtable
		{
			template<class DescriptorEditor>
			explicit vtable(std::type_identity<DescriptorEditor>)
				:create_table{[](
					void* handle,
					std::u8string_view label,
					widget_orientation orientation,
					std::initializer_list<char8_t const*> field_names
				){
					return static_cast<DescriptorEditor*>(handle)->create_table(
						label,
						orientation,
						field_names
					);
				}}
			{}

			descriptor_editor_ref (*create_table)(
				void*,
				std::u8string_view,
				widget_orientation,
				std::initializer_list<char8_t const*>
			);
		};

		resource_reference<vtable> m_handle;
	};
}

#endif