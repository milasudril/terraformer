//@	{"dependencies_extra":[{"ref":"./utils.o", "rel":"implementation"}]}

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
#include <ranges>
#include <cstdio>
#include <stdexcept>
#include <functional>
#include <string_view>

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

	template<class T>
	using empty = std::type_identity<T>;

	template<class First, class ... Types>
	struct pick_first
	{
		using type = First;
	};

	template<class ... Types>
	using pick_first_t = typename pick_first<Types...>::type;


	template<class T>
	requires std::is_arithmetic_v<T>
	inline std::string to_string_helper(T value)
	{
		std::array<char, 32> buffer{};
		std::to_chars(std::begin(buffer), std::end(buffer), value);
		return std::string{std::data(buffer)};
	}

	void bytes_to_hex(char* __restrict__ dest,  void const* __restrict__ src, size_t dest_length);

	inline std::string bytes_to_hex(void const* src, size_t src_length)
	{
		std::string ret(2*src_length, '0');
		bytes_to_hex(std::data(ret), src, std::size(ret));
		return ret;
	}

	struct hex_to_bytes_result
	{
		const char* ptr;
	};

	hex_to_bytes_result hex_to_bytes(void* __restrict__ dest, char const* __restrict__ src, size_t dest_length);

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

	inline float round_to_n_digits(float x, int n)
	{
		auto const scale = std::exp2(static_cast<float>(n) - std::ceil(std::log2(std::abs(x))));
		return std::round(x * scale) / scale;
	}

	inline float ceil_to_n_digits(float x, int n)
	{
		auto const scale = std::exp2(static_cast<float>(n) - std::ceil(std::log2(std::abs(x))));
		return std::ceil(x * scale) / scale;
	}

	inline uint32_t round_to_odd(float x)
	{
		return static_cast<uint32_t>(2.0f*std::floor(0.5f*x) + 1);
	}

	template<std::ranges::forward_range R1, std::ranges::forward_range R2, class Pred>
	auto find_matching_pair(R1&& r_1, R2&& r_2, Pred pred)
	{
		auto const end_1 = std::end(r_1);
		auto const end_2 = std::end(r_2);

		auto i_1 = std::begin(r_1);
		while(i_1 != end_1)
		{
			auto i_2 = std::begin(r_2);
			while(i_2 != end_2)
			{
				if(pred(*i_1, *i_2))
				{ return std::pair{i_1, i_2}; }
				++i_2;
			}
			++i_1;
		}
		return std::pair{end_1, end_2};
	}

	template<std::integral Dest, std::integral Src>
	Dest narrowing_cast(Src src)
	{
		auto result = static_cast<Dest>(src);
		if(static_cast<Src>(result) != src)
		{ throw std::runtime_error{"Value out of range"}; }

		return result;
	}

	template<class... Args, class RetType, class Class>
	constexpr auto resolve_overload(RetType (Class::*func)(Args...))
	{
    return func;
	}

	template<class... Args, class RetType, class Class>
	constexpr auto resolve_overload(RetType (Class::*func)(Args...) const)
	{
    return func;
	}

	template<class A, class B>
	constexpr bool compare_with_fallback(A const& a, B const& b)
	{
		if constexpr(std::equality_comparable_with<A, B>)
		{ return a == b; }
		else
		{ return false;}
	}

	constexpr bool is_valid(char32_t codepoint)
	{
		return codepoint <= 0x10ffff && (codepoint < 0xd800 || codepoint > 0xdfff);
	}

	template<class T, class U>
	requires std::is_convertible_v<U, U>
	constexpr void clamped_increment(T& x, U maxval)
	{
		if(x < static_cast<T>(maxval))
		{ ++x; }
	}

	template<class T, class U>
	requires std::is_convertible_v<U, T>
	constexpr void clamped_decrement(T& x, U minval)
	{
		if(x > static_cast<T>(minval))
		{ --x; }
	}

	std::u8string to_utf8(std::u32string_view);

	std::u32string to_utf32(std::u8string_view);

	inline std::u32string to_utf32(std::string_view str)
	{
		std::u32string ret;
		ret.reserve(std::size(str));
		std::ranges::copy(str, std::back_inserter(ret));
		return ret;
	}

	std::string scientific_to_natural(std::string_view input);

	std::string siformat(float value, size_t n_digits);

	template<class... Ts>
	struct overload : Ts...
	{ using Ts::operator()...; };

	inline size_t ceil_div(size_t x, size_t y)
	{ return x / y + (x % y == 0? 0 : 1); }

}

#endif