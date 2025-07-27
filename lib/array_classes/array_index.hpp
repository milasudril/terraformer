#ifndef TERRAFORMER_ARRAY_INDEX_HPP
#define TERRAFORMER_ARRAY_INDEX_HPP

#include "lib/common/tuple.hpp"

#include <type_traits>
#include <cstddef>
#include <stdexcept>

namespace terraformer
{
	template<class T, class Rep = size_t>
	class array_size
	{
	public:
		constexpr array_size() noexcept = default;

		constexpr explicit array_size(Rep value) noexcept: m_value{value}{}

		constexpr auto get() const noexcept { return m_value; }

		constexpr auto operator<=>(array_size const&) const noexcept = default;

		constexpr array_size& operator+=(array_size other)
		{
			if(__builtin_add_overflow(m_value, other.get(), &m_value))
			{ throw std::runtime_error{"Arithmetic overflow"}; }

			return *this;
		}

		constexpr array_size& operator-=(array_size other)
		{
			if(__builtin_sub_overflow(m_value, other.get(), &m_value))
			{ throw std::runtime_error{"Arithmetic overflow"}; }

			return *this;
		}

		constexpr array_size& operator*=(Rep factor)
		{
			if(__builtin_mul_overflow(m_value, factor, &m_value))
			{ throw std::runtime_error{"Arithmetic overflow"}; }

			return *this;
		}

		template<class Other>
		constexpr explicit operator array_size<Other, Rep>() const noexcept
		{ return array_size<Other>{m_value}; }

		constexpr explicit operator Rep() const
		{ return m_value; }

	private:
		Rep m_value{};
	};

	template<class Rep = size_t>
	using byte_size = array_size<std::byte, Rep>;

	template<class T, class Rep>
	constexpr auto make_byte_size(array_size<T, Rep> size)
	{ return byte_size<size_t>{(sizeof(T)*size).get()}; }

	template<class T, class Rep>
	constexpr auto operator+(array_size<T, Rep> a, array_size<T, Rep> b)
	{ return a += b; }

	template<class T, class Rep>
	constexpr auto operator-(array_size<T, Rep> a, array_size<T, Rep> b)
	{ return a -= b; }

	template<class T, class Rep>
	constexpr auto operator*(Rep c, array_size<T, Rep> a)
	{ return a *= c; }

	template<class T>
	class index_range
	{
	public:
		class iterator
		{
		public:
			explicit iterator(T current):m_current{current}{}

			constexpr iterator operator++(int) noexcept
			{
				auto tmp = *this;
				++m_current;
				return tmp;
			}

			constexpr iterator& operator++() noexcept
			{
				++m_current;
				return *this;
			}

			constexpr auto operator<=>(index_range::iterator const&) const noexcept = default;

			constexpr T operator*() const noexcept
			{ return m_current; }

			constexpr auto operator+(size_t offset) const
			{ return m_current + offset; }

		private:
			T m_current;
		};

		constexpr index_range() = default;

		constexpr explicit index_range(T starts_at, T bound) noexcept:
			m_starts_at{starts_at},
			m_bound{bound}
		{}

		constexpr auto begin() const noexcept
		{ return iterator{m_starts_at}; }

		constexpr auto end() const noexcept
		{ return iterator{m_bound}; }

		constexpr bool operator==(index_range const&) const noexcept = default;
		constexpr bool operator!=(index_range const&) const noexcept = default;

		constexpr bool empty() const noexcept
		{ return m_starts_at == m_bound; }

		constexpr auto size() const noexcept
		{ return m_bound - m_starts_at; }

		constexpr auto front() const noexcept
		{ return m_starts_at; }

		constexpr auto back() const noexcept
		{ return m_bound - 1; }

		constexpr auto bound() const noexcept
		{ return m_bound; }

		constexpr auto starts_at() const noexcept
		{ return m_starts_at; }

	private:
		T m_starts_at{};
		T m_bound{};
	};

	template<class T, class Rep = size_t>
	class array_index
	{
	public:
		using offset_type = std::make_signed_t<Rep>;
		using difference_type = std::make_signed_t<Rep>;
		using rep = Rep;
		using tag = T;

		constexpr array_index() noexcept = default;

		constexpr explicit array_index(array_size<T, Rep> value) noexcept: m_value{value}{}

		constexpr explicit array_index(Rep value) noexcept: m_value{value}{}

		constexpr auto get() const noexcept { return m_value; }

		constexpr auto& operator+=(offset_type other) noexcept
		{
			m_value += other;
			return *this;
		}

		constexpr auto& operator-=(offset_type other) noexcept
		{
			m_value -= other;
			return *this;
		}

		constexpr auto& operator++() noexcept
		{
			++m_value;
			return *this;
		}

		constexpr auto operator++(int) noexcept
		{
			auto const tmp = *this;
			++m_value;
			return tmp;
		}

		constexpr auto& operator--() noexcept
		{
			--m_value;
			return *this;
		}

		constexpr auto operator--(int) noexcept
		{
			auto const tmp = *this;
			--m_value;
			return tmp;
		}

		constexpr auto operator<=>(array_index const&) const noexcept = default;

		template<class Other>
		requires requires { {Other::template match_tag<T>()}; }
		constexpr explicit operator array_index<Other, Rep>() const noexcept
		{ return array_index<Other>{m_value}; }

		constexpr explicit operator array_size<T, Rep>() const noexcept
		{ return array_size<T, Rep>{m_value}; }

		constexpr bool within(index_range<array_index> range) const noexcept
		{ return *this >= range.starts_at() && *this < range.bound(); }

	private:
		Rep m_value{};
	};

	template<class T, class Rep>
	constexpr auto operator+(array_index<T, Rep> base, typename array_index<T, Rep>::offset_type offset) noexcept
	{ return base += offset; }

	template<class T, class Rep>
	constexpr auto operator-(array_index<T, Rep> base, typename array_index<T, Rep>::offset_type offset) noexcept
	{ return base -= offset; }

	template<class T, class Rep>
	constexpr auto operator-(array_index<T, Rep> a, array_index<T, Rep> b) noexcept
	{
		using offset_type = typename array_index<T, Rep>::offset_type;
		return static_cast<offset_type>(a.get()) - static_cast<offset_type>(b.get());
	}

	template<class T, class Rep>
	constexpr auto& deref(T* ptr, array_index<T, Rep> index) noexcept
	{ return ptr[index.get()]; }

	template<class T, class Rep>
	constexpr auto& deref(T const* ptr, array_index<T, Rep> index) noexcept
	{ return ptr[index.get()]; }

	template<class T, class Rep>
	constexpr auto operator<(array_index<T, Rep> index, array_size<T, Rep> size) noexcept
	{ return index.get() < size.get(); }

	template<class T, class Rep>
	constexpr auto operator!=(array_index<T, Rep> index, array_size<T, Rep> size) noexcept
	{ return index.get() != size.get(); }

	template<class T, class Rep>
	constexpr auto operator==(array_index<T, Rep> index, array_size<T, Rep> size) noexcept
	{ return index.get() == size.get(); }

	template<class T, class Rep = size_t>
	constexpr auto as_index(T* base, T* offset) noexcept
	{ return array_index<T, Rep>{static_cast<Rep>(std::distance(base, offset))}; }

	template<class Pointer>
	requires requires { typename Pointer::index_type; }
	constexpr auto as_index(Pointer base, Pointer offset) noexcept
	{
		using Rep = typename Pointer::index_type::rep;
		return typename Pointer::index_type{static_cast<Rep>(std::distance(base, offset))};

	}
}
#endif
