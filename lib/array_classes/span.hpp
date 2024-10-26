#ifndef TERRAFORMER_SPAN_HPP
#define TERRAFORMER_SPAN_HPP

#include "./array_index.hpp"

namespace terraformer
{
	template<class T, class IndexType = array_index<std::remove_const_t<T>>, class SizeType = array_size<std::remove_const_t<T>>>
	class span
	{
	public:
		using index_type = IndexType;
		using size_type = SizeType;

		explicit span() = default;

		explicit span(T* begin, T* end):m_begin{begin}, m_end{end}
		{}

		constexpr auto begin() const
		{ return m_begin; }

		constexpr auto end() const
		{ return m_end; }

		auto element_indices(size_t skip = 0) const
		{ return index_range{index_type{} + skip, index_type{} + size().get()}; }

		constexpr auto size() const
		{ return size_type{static_cast<size_t>(m_end - m_begin)}; }

		auto empty() const
		{ return begin() == end(); }

		constexpr auto& operator[](index_type index) const
		{ return m_begin[index.get()]; }

		constexpr auto data() const
		{ return m_begin; }

		constexpr operator span<T, array_index<T>, array_size<T>>() const
		{	return span<T, array_index<T>, array_size<T>>{m_begin, m_end}; }

		template<class Dummy = void>
		requires std::is_const_v<T>
		constexpr operator span<T, array_index<std::remove_const_t<T>>, array_size<std::remove_const_t<T>>>() const
		{ return decay(); }

		constexpr auto decay() const
		{
			return span<T, array_index<std::remove_const_t<T>>, array_size<std::remove_const_t<T>>>{m_begin, m_end};
		}

		constexpr auto& front() const
		{ return *m_begin; }

		constexpr auto& back() const
		{ return *(m_end - 1); }

	private:
		T* m_begin = nullptr;
		T* m_end = nullptr;
	};
}

#endif