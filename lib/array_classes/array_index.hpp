#ifndef TERRAFORMER_ARRAY_INDEX_HPP
#define TERRAFORMER_ARRAY_INDEX_HPP

#include <type_traits>
#include <cstddef>
#include <stdexcept>

namespace terraformer
{
	template<class T, class Rep = size_t>
	class array_size
	{
	public:
		constexpr array_size() = default;

		constexpr explicit array_size(Rep value): m_value{value}{}

		constexpr auto get() const { return m_value; }

		constexpr auto operator<=>(array_size const&) const = default;

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
		explicit operator array_size<Other, Rep>() const
		{ return array_size<Other>{m_value}; }

	private:
		Rep m_value{};
	};

	template<class Rep = size_t>
	using byte_size = array_size<std::byte, Rep>;

	template<class T, class Rep>
	constexpr auto make_byte_size(array_size<T, Rep> size)
	{ return byte_size{(sizeof(T)*size).get()}; }

	template<class T, class Rep>
	constexpr auto operator+(array_size<T, Rep> a, array_size<T, Rep> b)
	{ return a += b; }

	template<class T, class Rep>
	constexpr auto operator-(array_size<T, Rep> a, array_size<T, Rep> b)
	{ return a -= b; }

	template<class T, class Rep>
	constexpr auto operator*(Rep c, array_size<T, Rep> a)
	{ return a *= c; }

	template<class T, class Rep = size_t>
	class array_index
	{
	public:
		using offset_type = std::make_signed_t<Rep>;

		constexpr array_index() = default;

		constexpr explicit array_index(Rep value): m_value{value}{}

		constexpr auto get() const { return m_value; }

		constexpr auto& operator+=(offset_type other)
		{
			m_value += other;
			return *this;
		}

		constexpr auto& operator-=(offset_type other)
		{
			m_value -= other;
			return *this;
		}

		constexpr auto& operator++()
		{
			++m_value;
			return *this;
		}

		constexpr auto operator++(int)
		{
			auto const tmp = *this;
			++m_value;
			return tmp;
		}

		constexpr auto& operator--()
		{
			--m_value;
			return *this;
		}

		constexpr auto operator--(int)
		{
			auto const tmp = *this;
			--m_value;
			return tmp;
		}

		constexpr auto operator<=>(array_index const&) const = default;

	private:
		Rep m_value{};
	};

	template<class T, class Rep>
	constexpr auto operator+(array_index<T, Rep> base, typename array_index<T, Rep>::offset_type offset)
	{ return base += offset; }

	template<class T, class Rep>
	constexpr auto operator-(array_index<T, Rep> base, typename array_index<T, Rep>::offset_type offset)
	{ return base -= offset; }

	template<class T, class Rep>
	constexpr auto& deref(T* ptr, array_index<T, Rep> index)
	{ return ptr[index.get()]; }

	template<class T, class Rep>
	constexpr auto& deref(T const* ptr, array_index<T, Rep> index)
	{ return ptr[index.get()]; }

	template<class T, class Rep>
	constexpr auto operator<(array_index<T, Rep> index, array_size<T, Rep> size)
	{ return index.get() < size.get(); }

	template<class T, class Rep>
	constexpr auto operator!=(array_index<T, Rep> index, array_size<T, Rep> size)
	{ return index.get() != size.get(); }

}
#endif
