#ifndef TERRAFORMER_UTILS_HPP
#define TERRAFORMER_UTILS_HPP

#include <type_traits>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <numbers>
#include <algorithm>
#include <charconv>
#include <string>

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
		auto const xi = 2.0f*(0.5f - mod(x/(2.0f*std::numbers::pi_v<float>), 1.0f));
		return -(xi*(xi - 1.0f)*(xi + 1.0f)
			*((13968.0f/31680.0f)*(xi*xi)*(xi*xi) - (63304.0f/31680.0f)*(xi*xi) + 99433.0f/31680.0f));
	}

	constexpr auto mod(int32_t x, int32_t denom)
	{ return ((x % denom) + denom) % denom; }

	constexpr auto smoothstep(float x)
	{
		x = std::clamp(x, -1.0f, 1.0f);
		return (2.0f - x)*(x + 1.0f)*(x + 1.0f)/4.0f;
	}

	template<class T>
	using empty = std::type_identity<T>;

	template<class T>
	requires std::is_arithmetic_v<T>
	inline std::string to_string_helper(T value)
	{
		std::array<char, 32> buffer{};
		std::to_chars(std::begin(buffer), std::end(buffer), value);
		return std::string{std::data(buffer)};
	}

	template<class Context, class Callable, class ... Args>
	decltype(auto) try_and_catch(Context&& context, Callable&& func, Args&&... args)
	{
		try
		{ return std::invoke(std::forward<Callable>(func), std::forward<Args>(args)...); }
		catch(std::runtime_error const& error)
		{ std::forward<Context>(context)(error); }
	}

	inline void log_error(char const* msg)
	{
		fprintf(stderr, "(x) %s\n", msg);
		fflush(stderr);
	}
}

#endif