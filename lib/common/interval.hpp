#include "./utils.hpp"

#include <concepts>

namespace terraformer
{
	template<class IntervalType, class T>
	concept interval = std::totally_ordered<T> && requires(IntervalType x, T value)
	{
		{IntervalType::lower_bound_char};
		{IntervalType::upper_bound_char};
		{x.min} -> std::convertible_to<T>;
		{x.max} -> std::convertible_to<T>;
	};

	template<class I, class T = typename I::value_type>
	requires interval<I, T>
	std::string to_string(I const& range)
	{
		std::string ret{};
		ret += I::lower_bound_char;
		ret.append(to_string_helper(range.min)).append(", ").append(to_string_helper(range.max));
		ret += I::upper_bound_char;
		return ret;
	}

	template<class T>
	struct open_open_interval
	{
		using value_type = T;
		T min;
		T max;
		static constexpr auto lower_bound_char = ']';
		static constexpr auto upper_bound_char = '[';
	};

	template<class T>
	bool within(open_open_interval<T> range, T val)
	{ return val > range.min && val < range.max; }

	template<class T>
	struct closed_open_interval
	{
		using value_type = T;
		T min;
		T max;
		static constexpr auto lower_bound_char = '[';
		static constexpr auto upper_bound_char = '[';
	};

	template<class T>
	bool within(closed_open_interval<T> range, T val)
	{ return val >= range.min && val < range.max; }

	template<class T>
	struct open_closed_interval
	{
		using value_type = T;
		T min;
		T max;
		static constexpr auto lower_bound_char = ']';
		static constexpr auto upper_bound_char = ']';
	};

	template<class T>
	bool within(open_closed_interval<T> range, T val)
	{ return val > range.min && val <= range.max; }

	template<class T>
	struct closed_closed_interval
	{
		using value_type = T;
		T min;
		T max;
		static constexpr auto lower_bound_char = '[';
		static constexpr auto upper_bound_char = ']';
	};

	template<class T>
	bool within(closed_closed_interval<T> range, T val)
	{ return val >= range.min && val <= range.max; }
}