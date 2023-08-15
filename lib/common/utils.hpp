#ifndef TERRAFORMER_LIB_UTILS_HPP
#define TERRAFORMER_LIB_UTILS_HPP

#include <type_traits>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <numbers>

namespace terraformer
{
	template<class T, class U>
	concept same_as_unqual = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

	template<class T>
	constexpr T const* as_ptr_to_const(T* ptr)
	{ return ptr; }

	template<class MapType, class MappedType>
	concept map_2d = requires(MapType f, uint32_t x, uint32_t y)
	{
		{f(x, y)} -> std::convertible_to<MappedType>;
	};

	constexpr auto fast_fmod(float val, float divisor)
	{
		auto const reciprocal = 1.0f/divisor;
		return val - divisor * static_cast<int>(val * reciprocal);
	}

	constexpr auto mod(float x, float denom)
	{
		auto const ret = fast_fmod(x, denom);
		auto const alt_ret = ret + denom;
		return ret < 0.0f ? (alt_ret > 0.0f ? alt_ret : 0.0f) : ret;
	}

	constexpr auto approx_sine(float x)
	{
	#if 0
		auto const xi = 2.0f*(0.5f - mod(x/(2.0f*std::numbers::pi_v<float>), 1.0f));
		return 4.0f*xi*(xi - 1.0f)*(xi + 1.0f)*(4.0f*xi*xi - 7.0f)/9.0f;
	#else
		auto const xi = 2.0f*(0.5f - mod(x/(2.0f*std::numbers::pi_v<float>), 1.0f));
		return -(xi*(xi - 1.0f)*(xi + 1.0f)
			*((13968.0f/31680.0f)*(xi*xi)*(xi*xi) - (63304.0f/31680.0f)*(xi*xi) + 99433.0f/31680.0f));
	#endif
	}

	constexpr auto mod(int32_t x, int32_t denom)
	{
		return ((x % denom) + denom) % denom;
	}

	template<class T>
	using empty = std::type_identity<T>;
}

#endif