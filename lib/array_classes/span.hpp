#ifndef TERRAFORMER_SPAN_HPP
#define TERRAFORMER_SPAN_HPP

#include "./array_index.hpp"

namespace terraformer
{
	template<class T, class IndexType = array_index<T>, class SizeType = array_size<T>>
	class span
	{
	public:
		using index_type = IndexType;
		using size_type = SizeType;

		explicit span(T* begin, T* end):m_begin{begin}, m_end{end}
		{}

		constexpr auto begin() const
		{ return m_begin; }

		constexpr auto end() const
		{ return m_end; }

		constexpr auto first_element_index() const
		{ return index_type{}; }

		constexpr auto last_element_index() const
		{ return index_type{(size() - size_type{1}).get()}; }

		constexpr auto size() const
		{ return size_type{static_cast<size_t>(m_end - m_begin)}; }

		auto empty() const
		{ return size() == 0; }

		constexpr auto& operator[](index_type index) const
		{ return m_begin[index.get()]; }

		constexpr auto data() const
		{ return m_begin; }

		constexpr operator span<T, array_index<T>, array_size<T>>() const
		{ return span<T, array_index<T>, array_size<T>>{m_begin, m_end}; }

		constexpr auto& front() const
		{ return *m_begin; }

	private:
		T* m_begin;
		T* m_end;
	};
}

#endif