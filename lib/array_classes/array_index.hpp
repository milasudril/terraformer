#ifndef TERRAFORMER_ARRAY_INDEX_HPP
#define TERRAFORMER_ARRAY_INDEX_HPP

#include <type_traits>
#include <cstddef>

namespace terraformer
{
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
}

#endif