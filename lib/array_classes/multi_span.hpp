#ifndef TERRAFORMER_MULTI_SPAN_HPP
#define TERRAFORMER_MULTI_SPAN_HPP

#include "./span.hpp"
#include "lib/common/tuple.hpp"

#include <type_traits>
#include <cassert>

namespace terraformer
{
	template<class ... T>
	struct multi_array_tag
	{
		template<class U>
		requires std::disjunction_v<std::is_same<U, T>...>
		static consteval auto match_tag()
		{ return std::type_identity<U>{}; }

		static consteval void convert(T const&...){}

		static constexpr auto type_count = sizeof...(T);
	};

	template<size_t StartAt, class ArrayTag, class StorageTuple, class Head, class ... Args>
	void assign_values(
		StorageTuple const& dest,
		array_index<ArrayTag> offset,
		Head&& head,
		Args&&... args
	)
	{
		if constexpr(StartAt != ArrayTag::type_count)
		{
			auto obj = dest.template get<StartAt>() + offset.get();
			*obj = std::forward<Head>(head);
			if constexpr(sizeof...(args) != 0)
			{ assign_values<StartAt + 1>(dest, offset, std::forward<Args>(args)...); }
		}
	}

	template<class ... T>
	class multi_span
	{
	public:
		using storage_type = tuple<T*...>;
		using size_type = array_size<multi_array_tag<std::remove_const_t<T>...>>;
		using index_type = array_index<multi_array_tag<std::remove_const_t<T>...>>;
		using reference = tuple<T&...>;

		template<size_t Index>
		using attribute_type = std::tuple_element_t<Index, tuple<T...>>;

		multi_span() noexcept = default;

		explicit multi_span(T*... pointers, size_t size) noexcept:
			m_storage{pointers...},
			m_size{size}
		{}

		explicit multi_span(T*... pointers, size_type size) noexcept:
			m_storage{pointers...},
			m_size{size}
		{}

		explicit multi_span(storage_type const& storage, size_t size) noexcept:
			m_storage{storage},
			m_size{size}
		{}

		explicit multi_span(storage_type const& storage, size_type size) noexcept:
			m_storage{storage},
			m_size{size}
		{}

		template<class = void>
		requires(... && std::is_const_v<T>)
		explicit multi_span(multi_span<std::remove_const_t<T>...> span):
			m_storage{terraformer::apply([](auto const&... args){
				return tuple{static_cast<T*>(args)...};
			}, span.pointers())},
			m_size{span.size()}
		{}

		constexpr auto first_element_index() const noexcept
		{ return index_type{}; }

		constexpr auto last_element_index() const noexcept
		{ return index_type{(m_size - size_type{1}).get()}; }

		auto size() const noexcept
		{ return m_size; }

		auto empty() const noexcept
		{ return m_size.get() == 0; }

		template<size_t From, class ... Arg>
		void assign(index_type index, Arg&&... args)
		{ assign_values<From>(m_storage, index, std::forward<Arg>(args)...); }

		template<size_t AttributeIndex>
		auto get() const noexcept
		{
			using sel_attribute_type = attribute_type<AttributeIndex>;
			auto const ptr = m_storage.template get<AttributeIndex>();
			return span<sel_attribute_type, index_type, size_type>{ptr, ptr + m_size.get()};
		}

		reference operator[](index_type index) const noexcept
		{
			assert(index < m_size);
			return terraformer::apply(
				[index](auto const&... args) {
					return tuple<T&...>{*(args + index.get())...};
				},
				m_storage
			);
		}

		auto const& pointers() const noexcept
		{ return m_storage; }

	private:
		storage_type m_storage{};
		size_type m_size{};
	};

	template<class ... T>
	multi_span(T*..., size_t) -> multi_span<T...>;

	template<class ... T>
	multi_span(tuple<T*...>, size_t) -> multi_span<T...>;

	template<class>
	struct multi_span_const;

	template<class ... T>
	struct multi_span_const<multi_span<T...>>
	{
		using type = multi_span<T const...>;
	};

	template<class ... T>
	using multi_span_const_t = multi_span_const<T...>::type;
}

#endif
