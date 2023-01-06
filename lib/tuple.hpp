#ifndef TERRAFORMER_LIB_TUPLE_HPP
#define TERRAFORMER_LIB_TUPLE_HPP

#include <type_traits>
#include <utility>

namespace terraformer
{
	namespace tuple_detail
	{
		template<std::size_t Index, class First, class ... Types>
		class tuple:
			public tuple<Index, First>,
			public tuple<Index + 1, Types...>
		{
			using b1 = tuple<Index, First>;
			using b2 = tuple<Index + 1, Types...>;

		public:
			template<class X, class ... Y>
			requires(std::is_same_v<X, First>)
			explicit tuple(X&& first, Y&&... other):
				b1{std::forward<X>(first)},
				b2{std::forward<Y>(other)...}
			{}

			bool operator==(tuple const&) const = default;
			bool operator!=(tuple const&) const = default;
		};

		template<std::size_t Index, class T>
		class tuple<Index, T>
		{
		public:
			bool operator==(tuple const&) const = default;
			bool operator!=(tuple const&) const = default;

			constexpr decltype(auto) get() &
			{ return m_value; }

			constexpr decltype(auto) get() const&
			{ return m_value; }

			constexpr decltype(auto) get() &&
			{ return std::move(m_value); }

			constexpr decltype(auto) get() const&&
			{ return std::move(m_value); }

			T m_value;
		};

		template<std::size_t Index, class First, class ... Args>
		struct element
		{
			using type = typename element<Index - 1, Args...>::type;
		};

		template<class First, class ... Args>
		struct element<0, First, Args ...>
		{
			using type = First;
		};

		template<std::size_t Index, class ... Args>
		using element_t = element<Index, Args ...>::type;

		static_assert(std::is_same_v<int, element_t<1, double, int>>);
	}

	template<class First, class ... Types>
	class tuple : private tuple_detail::tuple<0, First, Types...>
	{
		using base = tuple_detail::tuple<0, First, Types...>;

		template<std::size_t i>
		using base_class_from_index = tuple_detail::tuple<i, tuple_detail::element_t<i, First, Types ...>>;

	public:
		using base::base;

		bool operator==(tuple const&) const = default;
		bool operator!=(tuple const&) const = default;

		template<std::size_t index>
		constexpr decltype(auto) get() &
		{
			return static_cast<base_class_from_index<index>&>(*this).get();
		}

		template<std::size_t index>
		constexpr decltype(auto) get() const&
		{
			return static_cast<base_class_from_index<index> const&>(*this).get();
		}

		template<std::size_t index>
		constexpr decltype(auto) get() &&
		{
			return static_cast<base_class_from_index<index>&&>(std::move(*this)).get();
		}

		template<std::size_t index>
		constexpr decltype(auto) get() const&&
		{
			return static_cast<base_class_from_index<index> const&&>(std::move(*this)).get();
		}
	};
}

namespace std
{
	template<class ... Types>
	struct tuple_size<terraformer::tuple<Types ...>>
	{
		static constexpr size_t value = sizeof...(Types);
	};

	template<std::size_t Index, class ... Types>
	struct tuple_element<Index, terraformer::tuple<Types ...>>
	{
		using type = terraformer::tuple_detail::element_t<Index, Types...>;
	};
}

#endif