#ifndef TERRAFORMER_BOUNDED_VALUE_HPP
#define TERRAFORMER_BOUNDED_VALUE_HPP

#include "./utils.hpp"

#include <stdexcept>

namespace terraformer
{
	template<auto Interval, typename decltype(Interval)::value_type DefaultValue>
	class bounded_value
	{
	public:
		static_assert(within(Interval, DefaultValue));
		using value_type = decltype(DefaultValue);

		constexpr bounded_value(): m_val{DefaultValue}{}

		constexpr explicit bounded_value(value_type val):m_val{val}
		{
			if(!within(Interval, m_val))
			{ throw std::runtime_error{"Value is out of range"}; }
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
}
#endif