#ifndef TERRAFORMER_LIB_UTILS_HPP
#define TERRAFORMER_LIB_UTILS_HPP

#include <type_traits>
#include <cmath>
#include <concepts>
#include <cstdint>

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
		{f(x, y)} -> std::convertible_to<MappedType>;
	};

	constexpr auto mod(float x, float denom)
	{
		auto ret = std::fmod(x, denom);
		return ret < 0.0f ? std::max(0.0f, ret + denom) : ret;
	}

	constexpr auto mod(int32_t x, int32_t denom)
	{
		return ((x % denom) + denom) % denom;
	}

	template<class T>
	using empty = std::type_identity<T>;
}

#endif