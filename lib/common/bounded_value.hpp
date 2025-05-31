#ifndef TERRAFORMER_BOUNDED_VALUE_HPP
#define TERRAFORMER_BOUNDED_VALUE_HPP

#include "./utils.hpp"
#include "./input_error.hpp"

namespace terraformer
{
	struct clamp_tag{};

	template<
		auto Interval,
		typename decltype(Interval)::value_type DefaultValue = typename decltype(Interval)::value_type{}
	>
	class bounded_value
	{
	public:
		static_assert(within(Interval, DefaultValue));
		using value_type = decltype(DefaultValue);

		static constexpr auto accepts_value(value_type val)
		{ return within(Interval, val); }

		static constexpr auto min()
		{ return Interval.min(); }

		static constexpr auto max()
		{ return Interval.max(); }

		static constexpr auto default_value()
		{ return DefaultValue; }

		constexpr bounded_value(): m_val{DefaultValue}{}

		constexpr explicit bounded_value(value_type val, clamp_tag) requires
			requires(value_type val) {{Interval.clamp(val)} -> std::same_as<value_type>;}:
			m_val{Interval.clamp(val)}
		{}

		constexpr explicit bounded_value(value_type val):m_val{val}
		{
			if(!within(Interval, m_val))
			{ throw input_error{std::string{"Value is out of range. Expected range is "}.append(to_string(Interval))}; }
		}

		auto& operator=(value_type val)
		{
			if(!within(Interval, val))
			{ throw input_error{std::string{"Value is out of range. Expected range is "}.append(to_string(Interval))}; }
			m_val = val;
			return *this;
		}

		constexpr auto operator<=>(bounded_value const& other) const = default;

		constexpr auto operator<=>(value_type other) const
		{ return static_cast<value_type>(*this) <=> other; }

		constexpr operator value_type() const
		{ return m_val; }

	private:
		value_type m_val;
	};

	template<auto Interval, typename decltype(Interval)::value_type DefaultValue>
	auto to_string(bounded_value<Interval, DefaultValue> value)
	{
		using T = decltype(DefaultValue);
		return to_string_helper(static_cast<T>(value));
	}

	template<
		auto Interval,
		typename decltype(Interval)::value_type DefaultValue
	>
	inline constexpr auto lerp(
		bounded_value<Interval, DefaultValue> a,
		bounded_value<Interval, DefaultValue> b,
		typename decltype(Interval)::value_type t
	)
	{
		return bounded_value<Interval, DefaultValue>{std::lerp(a, b, t)};
	}
}
#endif