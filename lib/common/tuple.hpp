#ifndef TERRAFORMER_TUPLE_HPP
#define TERRAFORMER_TUPLE_HPP

#include "./utils.hpp"

#include <type_traits>
#include <utility>
#include <functional>
#include <tuple>
#include <string>

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
			constexpr tuple() = default;

			template<class X, class ... Y>
			constexpr explicit tuple(X&& first, Y&&... other):
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

			[[nodiscard]] constexpr T& get() &
			{ return m_value; }

			[[nodiscard]] constexpr T const& get() const&
			{ return m_value; }

			[[nodiscard]] constexpr T&& get() &&
			{ return std::move(m_value); }

			[[nodiscard]] constexpr T const&& get() const&&
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

		template <class F, class Tuple, std::size_t... I>
		constexpr decltype(auto) apply(F&& f, Tuple&& t, std::index_sequence<I...>)
		{
			return std::invoke(std::forward<F>(f), get<I>(std::forward<Tuple>(t))...);
		}

		template <size_t... Is, class TupleA, class TupleB>
		bool equal(std::index_sequence<Is...>, TupleA const& a, TupleB const& b)
		{
			return (... && (get<Is>(a) == get<Is>(b)));
		}
	}

	template<class ... Types>
	class tuple : private tuple_detail::tuple<0, Types...>
	{
		using base = tuple_detail::tuple<0, Types...>;

		template<std::size_t i>
		using base_class_from_index = tuple_detail::tuple<i, tuple_detail::element_t<i, Types ...>>;

	public:
		tuple() = default;

		struct enable_apply{};

		template<class ... U>
		requires(... && same_as_unqual<Types, U>)
		constexpr explicit tuple(U&&... t):base{std::forward<U>(t)...}{}

		constexpr tuple(tuple&&) = default;
		constexpr tuple(tuple const&) = default;
		constexpr tuple& operator=(tuple&&) = default;
		constexpr tuple& operator=(tuple const&) = default;

		bool operator==(tuple const&) const = default;
		bool operator!=(tuple const&) const = default;

		template<std::size_t index>
		[[nodiscard]] constexpr decltype(auto) get() &
		{ return static_cast<base_class_from_index<index>&>(*this).get(); }

		template<std::size_t index>
		[[nodiscard]] constexpr decltype(auto) get() const&
		{ return static_cast<base_class_from_index<index> const&>(*this).get(); }

		template<std::size_t index>
		[[nodiscard]] constexpr decltype(auto) get() &&
		{ return static_cast<base_class_from_index<index>&&>(*this).get(); }

		template<std::size_t index>
		[[nodiscard]] constexpr decltype(auto) get() const&&
		{ return static_cast<base_class_from_index<index> const&&>(std::move(*this)).get(); }
	};

	template<class ... Types>
	tuple(Types...)->tuple<Types ...>;

	template<std::size_t I, class ... Types>
	[[nodiscard]] decltype(auto) get(tuple<Types...>& t)
	{ return t.template get<I>(); }

	template<std::size_t I, class ... Types>
	[[nodiscard]] decltype(auto) get(tuple<Types...> const& t)
	{ return t.template get<I>(); }

	template<std::size_t I, class ... Types>
	[[nodiscard]]  decltype(auto) get(tuple<Types...>&& t)
	{ return std::move(t).template get<I>(); }

	template<std::size_t I, class ... Types>
	[[nodiscard]] decltype(auto) get(tuple<Types...> const&& t)
	{ return std::move(t).template get<I>();}

	template<class T>
	concept supports_apply = requires()
	{
		typename T::enable_apply;
	};

	template<class F, class Tuple>
	requires supports_apply<std::remove_reference_t<Tuple>>
	constexpr decltype(auto) apply(F&& f, Tuple&& t)
	{
		return tuple_detail::apply(
			std::forward<F>(f), std::forward<Tuple>(t),
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
	}

	template<class ... Types>
	[[nodiscard]] constexpr bool operator==(tuple<Types const&...> const& a, tuple<Types...> const& b)
	{
		return tuple_detail::equal(std::make_index_sequence<sizeof...(Types)>{}, a, b);
	}

	template<class ... Types>
	[[nodiscard]] constexpr bool operator==(tuple<Types...> const& a,
		tuple<Types const&...> const& b)
	{
		return b == a;
	}

	template<class ... Types>
	[[nodiscard]] constexpr bool operator!=(tuple<Types const&...> const& a,
		tuple<Types...> const& b)
	{
		return !(a == b);
	}

	template<class ... Types>
	[[nodiscard]] constexpr bool operator!=(tuple<Types...> const& a,
		tuple<Types const&...> const& b)
	{
		return !(b == a);
	}

	template <class...>
	inline constexpr auto is_unique_v = std::true_type{};

	template <class T, class... Rest>
	inline constexpr auto is_unique_v<T, Rest...> = std::bool_constant<(!std::is_same_v<T, Rest> && ...) && is_unique_v<Rest...>>{};

	template <class T, size_t Index, class Head, class ... Tail>
	constexpr auto get_index_from_type_impl()
	{
		if constexpr (std::is_same_v<T, Head>)
		{ return Index; }
		else
		if constexpr (sizeof...(Tail) == 0)
		{ return Index + 1; }
		else
		{ return get_index_from_type_impl<T, Index + 1, Tail...>(); }
	}

	template <class T, class... Args>
	constexpr auto get_index_from_type()
	{
		static_assert(is_unique_v<Args...>, "Type not unique");
		return get_index_from_type_impl<T, 0, Args...>();
	}

	template<class T, class ... Args>
	constexpr auto get_value_from_type(tuple<Args...> const& t)
	{
		return t.template get<get_index_from_type<T, Args...>()>();
	}

	template<class T, class U = T>
	using pair = std::pair<T, U>;
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