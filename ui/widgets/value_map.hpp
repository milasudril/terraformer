#ifndef TERRAFORMER_UI_WIDGETS_VALUE_MAP_HPP
#define TERRAFORMER_UI_WIDGETS_VALUE_MAP_HPP

#include "lib/common/unique_resource.hpp"

namespace terraformer::ui::widgets
{
	struct value_map_vtable
	{
		template<class ValueMap>
		constexpr explicit value_map_vtable(std::type_identity<ValueMap>):
			from_value{[](void const* obj, float value){
				return static_cast<ValueMap const*>(obj)->from_value(value);
			}},
			to_value{[](void const* obj, float value){
				return static_cast<ValueMap const*>(obj)->to_value(value);
			}}
		{}

		float (*from_value)(void const*, float);
		float (*to_value)(void const*, float);
	};

	using type_erased_value_map = unique_resource<value_map_vtable>;
}

#endif