#ifndef TERRAFORMER_SPAN_HPP
#define TERRAFORMER_SPAN_HPP

#include "./array_index.hpp"

#include <ranges>

namespace terraformer
{
	struct clamp_index{};

	template<class T, class IndexType = array_index<std::remove_const_t<T>>, class SizeType = array_size<std::remove_const_t<T>>>
	class span: public std::ranges::view_base
	{
	public:
		using index_type = IndexType;
		using offset_type = IndexType::offset_type;
		using size_type = SizeType;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		constexpr span() = default;

		constexpr explicit span(T* begin, T* end):m_begin{begin}, m_end{end}
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

		constexpr auto& operator()(typename index_type::offset_type index, clamp_index) const
		{
			return (*this)[
				index_type{} + std::clamp(index, offset_type{}, static_cast<offset_type>(size().decay()) - 1)
			];
		}

		template<class U>
		T value_or(index_type index, U&& default_value) const noexcept
		{
			if(index < size())
			{ return (*this)[index]; }

			return T{std::forward<U>(default_value)};
		}

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
		T* __restrict__ m_begin = nullptr;
		T* __restrict__ m_end = nullptr;
	};

	enum class span_search_direction{backwards = -1, stay = 0, forwards = 1};

	template<class T, class IndexType, class SizeType, class Predicate>
	IndexType find_next_wrap_around(
		span<T, IndexType, SizeType> span,
		IndexType start_offset,
		span_search_direction dir,
		Predicate pred
	)
	{
		if(span.empty()) [[unlikely]]
		{ return decltype(span)::npos; }

		if(dir == span_search_direction::stay)
		{ return start_offset; }

		auto const step = (dir == span_search_direction::backwards)? -1 : 1;
		auto const indices = span.element_indices();
		auto const last_index = (dir == span_search_direction::backwards)?
			indices.front() :
			indices.back();

		auto const first_index = (dir == span_search_direction::backwards)?
			indices.back() :
			indices.front();

		auto offset = (start_offset == decltype(span)::npos)?
			first_index :
			((start_offset == last_index)? first_index : start_offset + step);

		for(size_t k = 0; k != std::size(span).get(); ++k)
		{
			if(pred(span[offset]))
			{ return offset; }

			offset = (offset == last_index)? first_index :  offset + step;
		}

		return decltype(span)::npos;
	}
}

template<class T, class IndexType, class SizeType>
constexpr bool std::ranges::enable_borrowed_range<terraformer::span<T, IndexType, SizeType>> = true;

#endif
