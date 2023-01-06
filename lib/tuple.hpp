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

		template<auto IndexType, class First, class ... Types>
		class tuple:
			private tuple<IndexType, First>,
			private tuple<next_index(IndexType), Types...>
		{
		public:
			bool operator==(tuple const&) const = default;
			bool operator!=(tuple const&) const = default;
		};

		template<auto IndexType, class T>
		class tuple<IndexType, T>
		{
		public:
			bool operator==(tuple const&) const = default;
			bool operator!=(tuple const&) const = default;

			constexpr T& get() &
			{ return m_value; }

			constexpr T const& get() const&
			{ return m_value; }

			constexpr T&& get() &&
			{ return std::move(m_value); }

			constexpr T const&& get() const&&
			{ return std::move(m_value); }

		private:
			T m_value;
		};
	}

	template<class IndexType, class First, class ... Types>
	class tuple : private tuple_detail::tuple<tuple_detail::first_index<IndexType>(), First, Types...>
	{
	public:
		bool operator==(tuple const&) const = default;
		bool operator!=(tuple const&) const = default;

		template<IndexType index>
		constexpr decltype(auto) get() &
		{
			return static_cast<tuple_detail::tuple<index, First, Types...>&>(*this).get();
		}

		template<IndexType index>
		constexpr decltype(auto) get() const&
		{
			return static_cast<tuple_detail::tuple<index, First, Types...> const&>(*this).get();
		}

		template<IndexType index>
		constexpr decltype(auto) get() &&
		{
			return static_cast<tuple_detail::tuple<index, First, Types...>&&>(*this).get();
		}

		template<IndexType index>
		constexpr decltype(auto) get() const&&
		{
			return static_cast<tuple_detail::tuple<index, First, Types...> const&&>(*this).get();
		}
	};
}

#endif