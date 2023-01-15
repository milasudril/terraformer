#ifndef TERRAFORMER_LIB_UTILS_HPP
#define TERRAFORMER_LIB_UTILS_HPP

#include <type_traits>
#include <cstdint>
#include <concepts>

namespace terraformer
{
	template<class T, class U>
	concept same_as_unqual = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

	template<class T>
	constexpr T const* as_ptr_to_const(T* ptr)
	{
		return ptr;
	}

	template<class MapType, class MappedType>
	concept map_2d = requires(MapType f, uint32_t x, uint32_t y)
	{
		typename MapType::mapped_type;
		{f(x, y)} -> std::same_as<MappedType>;
	} && std::is_same_v<MappedType, typename MapType::mapped_type>;

	static_assert(!map_2d<int, int>);
}

#endif