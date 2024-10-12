#ifndef TERRAFORMER_SPAN_HPP
#define TERRAFORMER_SPAN_HPP

#include "./array_index.hpp"

#include <ranges>

namespace terraformer
{
	template<class T, class IndexType = array_index<std::remove_const_t<T>>, class SizeType = array_size<std::remove_const_t<T>>>
	class span
	{
	public:
		using index_type = IndexType;
		using size_type = SizeType;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		explicit span() = default;

		explicit span(T* begin, T* end):m_begin{begin}, m_end{end}
		{}

		constexpr auto begin() const
		{ return m_begin; }

		constexpr auto end() const
		{ return m_end; }

		static constexpr auto first_element_index()
		{ return index_type{}; }

		constexpr auto last_element_index() const
		{ return index_type{(size() - size_type{1}).get()}; }

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
		requires (!std::is_const_v<T>)
		constexpr operator span<T const, array_index<T>, array_size<T>>() const
		{	return span<T const, array_index<T>, array_size<T>>{m_begin, m_end}; }

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

	enum class search_direction{backward = -1, forward = 1};

	template<class T, class IndexType, class SizeType, class Predicate>
	IndexType find_next_wrap_around(
		span<T, IndexType, SizeType> span,
		IndexType offset,
		search_direction dir,
		Predicate pred
	)
	{
		if(span.empty()) [[unlikely]]
		{ return decltype(span)::npos; }

		auto const step = (dir == search_direction::backward)? -1 : 1;

		auto const last_index = (dir == search_direction::backward)?
			span.first_element_index() :
			span.last_element_index();

		auto const first_index = (dir == search_direction::backward)?
			span.last_element_index() :
			span.first_element_index();

		for(auto k = span.first_element_index(); k != std::size(span); ++k)
		{
			if(pred(span[offset]))
			{ return offset; }

			if(offset == last_index) [[unlikely]]
			{ offset = first_index; }
			else
			{ offset += step; }
		}

		return decltype(span)::npos;
	}
}

template<class T, class IndexType, class SizeType>
constexpr bool std::ranges::enable_borrowed_range<terraformer::span<T, IndexType, SizeType>> = true;

#endif