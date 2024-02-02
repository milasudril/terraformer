#ifndef TERRAFORMER_SPAN_HPP
#define TERRAFORMER_SPAN_HPP

#include "./array_index.hpp"

namespace terraformer
{
	template<class T>
	class span
	{
	public:
		explicit span(T* begin, T* end):m_begin{begin}, m_end{end}
		{}

		constexpr auto begin() const
		{ return m_begin; }

		constexpr auto end() const
		{ return m_end; }

		constexpr auto first_element_index() const
		{ return array_index<T>{}; }

		constexpr auto size() const
		{ return array_size<T>{static_cast<size_t>(m_end - m_begin)}; }

		constexpr auto& operator[](array_index<T> index) const
		{ return m_begin[index.get()]; }

		constexpr auto data() const
		{ return m_begin; }

	private:
		T* m_begin;
		T* m_end;
	};
}

#endif