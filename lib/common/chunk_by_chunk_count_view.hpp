#ifndef TERRAFORMER_CHUNK_BY_CHUNK_COUNT_VIEW_HPP
#define TERRAFORMER_CHUNK_BY_CHUNK_COUNT_VIEW_HPP

#include <iterator>
#include <ranges>

namespace terraformer
{
	template<std::ranges::view View>
		requires std::ranges::random_access_range<View>
	class chunk_by_chunk_count_view
	{
	public:
		using inner_iterator = decltype(std::begin(std::declval<View>()));

		class iterator
		{
		public:
			explicit iterator(inner_iterator begin) noexcept:
				m_begin{begin},
				m_end{begin},
				m_chunk_size{},
				m_reminder{}
			{}

			explicit iterator(inner_iterator begin, size_t chunk_size, size_t reminder) noexcept:
				m_begin{begin},
				m_end{begin + ((reminder != 0)? 1 : 0) + chunk_size},
				m_chunk_size{chunk_size},
				m_reminder{reminder != 0? reminder - 1 : reminder}
			{}

			iterator& operator++() noexcept
			{
				auto const new_end = m_end + (((m_reminder != 0)? 1 : 0) + m_chunk_size);
				m_reminder = (m_reminder != 0)? m_reminder - 1 : m_reminder;
				m_begin = m_end;
				m_end = new_end;
				return *this;
			}

			iterator operator++(int) noexcept
			{
				auto tmp = *this;
				++this;
				return tmp;
			}

			constexpr bool operator==(iterator const& other) const noexcept
			{ return m_begin == other.m_begin; }

			constexpr bool operator!=(iterator const& other) const noexcept
			{ return !(*this == other); }

			constexpr auto operator*() const noexcept
			{
				return std::ranges::subrange{m_begin, m_end};
			}

		private:
			inner_iterator m_begin;
			inner_iterator m_end;
			size_t m_chunk_size;
			size_t m_reminder;
		};

		explicit chunk_by_chunk_count_view(View view, size_t chunk_count) noexcept:
			m_view{std::move(view)},
			m_chunk_count{chunk_count},
			m_chunk_size{std::size(view)/chunk_count},
			m_reminder{std::size(view)%chunk_count}
		{}

		auto begin() const noexcept
		{ return iterator{std::begin(m_view), m_chunk_size, m_reminder}; }

		auto end() const noexcept
		{ return iterator{std::end(m_view)}; }

		auto size() const noexcept
		{ return m_chunk_count; }

		auto reminder() const noexcept
		{ return m_reminder; }

		auto chunk_size() const noexcept
		{ return m_chunk_size; }

		auto base() const noexcept
		{ return m_view; }

	private:
		View m_view;
		size_t m_chunk_count;
		size_t m_chunk_size;
		size_t m_reminder;
	};
}
#endif