#ifndef TERRAFORMER_ARRAY_POINTER_HPP
#define TERRAFORMER_ARRAY_POINTER_HPP

#include "./array_index.hpp"

#include <type_traits>
#include <cstddef>

namespace terraformer
{
	template<class T, class IndexType = array_index<T>>
	class array_pointer
	{
	public:
		using index_type = IndexType;
		using value_type = T;

		constexpr array_pointer() = default;

		constexpr explicit array_pointer(T* ptr): m_value{ptr}{}

		constexpr auto get() const { return m_value; }

		constexpr auto operator<=>(array_pointer const&) const = default;

		constexpr array_pointer& operator+=(index_type offset)
		{
			m_value += offset.get();
			return *this;
		}

		constexpr array_pointer& operator-=(index_type offset)
		{
			m_value -= offset.get();
			return *this;
		}

		constexpr explicit operator T*() const
		{ return m_value; }

		T* operator->() const
		{ return m_value; }

		T& operator*() const
		{ return *m_value; }

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


	private:
		T* m_value{};
	};

	template<class T, class IndexType>
	constexpr auto operator+(array_pointer<T, IndexType> a, IndexType b)
	{ return a += b; }

	template<class T, class IndexType>
	constexpr auto operator-(array_pointer<T, IndexType> a, IndexType b)
	{ return a -= b; }

	template<class T, class IndexType>
	constexpr auto operator-(array_pointer<T, IndexType> a, array_pointer<T, IndexType> b)
	{ return static_cast<typename IndexType::rep>(a.get() - b.get()); }

}
#endif
