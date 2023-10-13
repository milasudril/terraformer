#ifndef TERRAFORMER_INTERVAL_HPP
#define TERRAFORMER_INTERVAL_HPP

#include "./utils.hpp"

#include <stdexcept>

#include <concepts>

namespace terraformer
{
	template<class IntervalType, class T>
	concept interval = std::totally_ordered<T> && requires(const IntervalType& x, T value)
	{
		{IntervalType::lower_bound_char};
		{IntervalType::upper_bound_char};
		{x.min()} -> std::convertible_to<T>;
		{x.max()} -> std::convertible_to<T>;
		{within(x, value)} -> std::convertible_to<bool>;
	};

	template<class I, class T = typename I::value_type>
	requires interval<I, T>
	std::string to_string(I const& range)
	{
		std::string ret{};
		ret += I::lower_bound_char;
		ret.append(to_string_helper(range.min())).append(", ").append(to_string_helper(range.max()));
		ret += I::upper_bound_char;
		return ret;
	}

	template<class T>
	class open_open_interval
	{
	public:
		using value_type = T;
		static constexpr auto lower_bound_char = ']';
		static constexpr auto upper_bound_char = '[';

		constexpr open_open_interval(T min, T max):m_min{min}, m_max{max}{}

		constexpr T min() const
		{ return m_min; }

		constexpr T max() const
		{ return m_max; }

	private:
		T m_min;
		T m_max;
	};

	template<class T>
	bool within(open_open_interval<T> range, T val)
	{ return val > range.min() && val < range.max(); }

	template<class T>
	struct closed_open_interval
	{
	public:
		using value_type = T;
		static constexpr auto lower_bound_char = '[';
		static constexpr auto upper_bound_char = '[';

		constexpr closed_open_interval(T min, T max):m_min{min}, m_max{max}{}

		constexpr T min() const
		{ return m_min; }

		constexpr T max() const
		{ return m_max; }

	private:
		T m_min;
		T m_max;
	};

	template<class T>
	bool within(closed_open_interval<T> range, T val)
	{ return val >= range.min() && val < range.max(); }

	template<class T>
	class open_closed_interval
	{
	public:
		using value_type = T;
		static constexpr auto lower_bound_char = ']';
		static constexpr auto upper_bound_char = ']';

		constexpr open_closed_interval(T min, T max):m_min{min}, m_max{max}{}

		constexpr T min() const
		{ return m_min; }

		constexpr T max() const
		{ return m_max; }

	private:
		T m_min;
		T m_max;
	};

	template<class T>
	bool within(open_closed_interval<T> range, T val)
	{ return val > range.min() && val <= range.max(); }

	template<class T>
	class closed_closed_interval
	{
	public:
		using value_type = T;
		static constexpr auto lower_bound_char = '[';
		static constexpr auto upper_bound_char = ']';

		constexpr closed_closed_interval(T min, T max):m_min{min}, m_max{max}{}

		constexpr T min() const
		{ return m_min; }

		constexpr T max() const
		{ return m_max; }

	private:
		T m_min;
		T m_max;
	};

	template<class T>
	bool within(closed_closed_interval<T> range, T val)
	{ return val >= range.min() && val <= range.max(); }
}
#endif