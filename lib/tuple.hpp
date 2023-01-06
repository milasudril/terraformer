#ifndef TERRAFORMER_LIB_TUPLE_HPP
#define TERRAFORMER_LIB_TUPLE_HPP

#include <type_traits>
#include <utility>

namespace terraformer
{
	namespace tuple_detail
	{
		template<class T>
		requires(std::is_integral_v<T>)
		constexpr T first_index()
		{
			return T{0};
		}

		template<class T>
		requires(!std::is_integral_v<T>)
		constexpr T first_index()
		{
			return first(std::type_identity<T>{});
		}

		template<class T>
		requires(std::is_integral_v<T>)
		constexpr T next_index(T val)
		{
			return val + 1;
		}

		template<class T>
		requires(!std::is_integral_v<T>)
		constexpr T next_index(T val)
		{
			return next_after(val);
		}

		template<class T>
		requires(std::is_integral_v<T>)
		constexpr T previous_index(T val)
		{
			return val - 1;
		}

		template<class T>
		requires(!std::is_integral_v<T>)
		constexpr T previous_index(T val)
		{
			return next_before(val);
		}

		template<auto Index, class First, class ... Types>
		class tuple:
			public tuple<Index, First>,
			public tuple<next_index(Index), Types...>
		{
			using b1 = tuple<Index, First>;
			using b2 = tuple<next_index(Index), Types...>;

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

		template<auto Index, class T>
		class tuple<Index, T>
		{
		public:
			bool operator==(tuple const&) const = default;
			bool operator!=(tuple const&) const = default;

			constexpr decltype(auto) get() &
			{ return m_value; }

			constexpr decltype(auto) get() const&
			{ return m_value; }
/*
			constexpr decltype(auto) get() &&
			{ return std::move(m_value); }

			constexpr decltype(auto) get() const&&
			{ return std::move(m_value); }*/

			T m_value;
		};

		template<std::size_t Index, class First, class ... Args>
		struct get_type_from_index
		{
			using type = typename get_type_from_index<previous_index(Index), Args...>::type;
		};

		template<class First, class ... Args>
		struct get_type_from_index<first_index<std::size_t>(), First, Args ...>
		{
			using type = First;
		};

		template<std::size_t Index, class First, class ... Args>
		using type = get_type_from_index<Index, First, Args ...>::type;
	}

	template<class Index, class First, class ... Types>
	class tuple : private tuple_detail::tuple<tuple_detail::first_index<Index>(), First, Types...>
	{
		using base = tuple_detail::tuple<tuple_detail::first_index<Index>(), First, Types...>;

		template<Index i>
		using base_class_from_index = tuple_detail::tuple<i, tuple_detail::type<i, First, Types ...>>;

	public:
		using base::base;

		bool operator==(tuple const&) const = default;
		bool operator!=(tuple const&) const = default;

		template<Index index>
		constexpr decltype(auto) get() &
		{
			return static_cast<base_class_from_index<index>&>(*this).get();
		}

		template<Index index>
		constexpr decltype(auto) get() const&
		{
			return static_cast<base_class_from_index<index> const&>(*this).get();
		}

		template<Index index>
		constexpr decltype(auto) get() &&
		{
			return static_cast<base_class_from_index<index>&&>(*this).get();
		}

		template<Index index>
		constexpr decltype(auto) get() const&&
		{
			return static_cast<base_class_from_index<index> const&&>(*this).get();
		}
	};
}

namespace std
{
	template<class Index, class First, class ... Types>
	struct tuple_size<terraformer::tuple<Index, First, Types ...>>
	{
		static constexpr size_t value = 1 + sizeof...(Types);
	};
}

#endif