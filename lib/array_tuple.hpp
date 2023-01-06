#ifndef TERRAFORMER_LIB_ARRAY_TUPLE_HPP
#define TERRAFORMER_LIB_ARRAY_TUPLE_HPP

#include <memory>
#include <tuple>
#include <algorithm>
#include <span>
#include <limits>
#include <concepts>
#include <cassert>

namespace terraformer
{
	namespace array_tuple_detail
	{
		template<class T, class U>
		concept same_as_unqual = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

		template <size_t... Is, class TupleLHS, class TupleRHS>
		void assign(std::index_sequence<Is...>, TupleLHS& l, size_t index, TupleRHS&& r)
		{
			(...,(
			std::get<Is>(l)[index] = std::forward<std::tuple_element_t<Is, TupleRHS>>(std::get<Is>(r))
			));
		}

		template <size_t... Is, class Tuple>
		void move_data(std::index_sequence<Is...>, Tuple&& from, Tuple& to, size_t size)
		{
			(...,(
			std::move(std::get<Is>(from).get(), std::get<Is>(from).get() + size, std::get<Is>(to).get())
			));
		}

		template <size_t... Is, class Tuple>
		void copy_data(std::index_sequence<Is...>, Tuple&& from, Tuple& to, size_t size)
		{
			(...,(
			std::copy(std::get<Is>(from).get(), std::get<Is>(from).get() + size, std::get<Is>(to).get())
			));
		}

		template <size_t... Is, class Tuple>
		bool equal(std::index_sequence<Is...>, Tuple const& a, Tuple const& b, size_t size)
		{
			return (... &&
			std::equal(std::get<Is>(a).get(), std::get<Is>(a).get() + size, std::get<Is>(b).get())
			);
		}
	}

	template<class ... Types>
	class array_tuple
	{
	public:
		using size_type = uint32_t;
		using value_type = std::tuple<Types...>;

		array_tuple():m_size{0},m_capacity{0}{}

		~array_tuple() = default;

		[[nodiscard]] array_tuple(array_tuple&& other) noexcept:
			m_size{other.size()},
			m_capacity{other.capacity()},
			m_storage{std::move(other.m_storage)}
		{
			other.m_size = 0;
			other.m_capacity = 0;
		}

		[[nodiscard]] array_tuple(array_tuple const& other):
			m_size{other.size()},
			m_capacity{other.capacity()},
			m_storage{std::tuple{std::make_unique_for_overwrite<Types[]>(other.capacity())...}}

		{
			copy_data(other.m_storage, m_storage, m_size);
		}

		void reserve(size_type n)
		{
			realloc(n);
		}

		array_tuple& operator=(array_tuple&& other) noexcept
		{
			m_size = std::exchange(other.m_size, 0);
			m_capacity = std::exchange(other.m_capacity, 0);
			m_storage = std::exchange(other.m_storage, nullptr);
			return *this;
		}

		array_tuple& operator=(array_tuple const& other)
		{
			array_tuple tmp{other};
			*this = std::move(tmp);
			return *this;
		}

		[[nodiscard]] bool operator==(array_tuple const& other) const
		{
			return m_size == other.size() &&
				array_tuple_detail::equal(m_storage, other.m_storage, m_size);
		}

		[[nodiscard]] bool operator!=(array_tuple const& other) const
		{ return !(*this == other); }

		template<class ... Vals>
		requires(array_tuple_detail::same_as_unqual<Types, Vals> && ...)
		void push_back(Vals&&... vals)
		{
			if(m_size == m_capacity) [[unlikely]]
			{realloc();}

			array_tuple_detail::assign(std::make_index_sequence<sizeof...(Types)>{},
				m_storage, m_size, std::tuple{std::forward<Vals>(vals)...});
			++m_size;
		}

		template<class Tuple>
		requires(array_tuple_detail::same_as_unqual<Tuple, value_type>)
		void push_back(Tuple&& t)
		{
			if(m_size == m_capacity) [[unlikely]]
			{realloc();}

			array_tuple_detail::assign(std::make_index_sequence<sizeof...(Types)>{},
				m_storage, m_size, std::forward<Tuple>(t));
			++m_size;
		}

		[[nodiscard]] bool empty() const
		{ return size() == 0; }

		[[nodiscard]] size_type size() const
		{ return m_size; }

		[[nodiscard]] size_type capacity() const
		{ return m_capacity; }

		template<size_t I>
		[[nodiscard]] auto get()
		{ return std::span{std::get<I>(m_storage).get(), m_size}; }

		template<size_t I>
		[[nodiscard]] auto get() const
		{ return std::span{std::as_const(std::get<I>(m_storage).get()), m_size}; }

		template<class Dummy>
		requires(std::is_trivially_copyable_v<value_type>)
		[[nodiscard]] auto operator[](size_type index) const
		{
			assert(index < m_size);
			return std::apply([index](auto const& ... items){
				return std::tuple{items[index]...};
			}, m_storage);
		}

		template<class ... Vals>
		requires(array_tuple_detail::same_as_unqual<Types, Vals> && ...)
		void assign(size_type index, Vals&&... vals)
		{
			assert(index < m_size);
			array_tuple_detail::assign(std::make_index_sequence<sizeof...(Types)>{},
				m_storage, index, std::tuple{std::forward<Vals>(vals)...});
		}

		template<class Tuple>
		requires(array_tuple_detail::same_as_unqual<Tuple, value_type>)
		void assign(size_type index, Tuple&& t)
		{
			assert(index < m_size);
			array_tuple_detail::assign(std::make_index_sequence<sizeof...(Types)>{},
				m_storage, index, std::forward<Tuple>(t));
		}

	private:
		void realloc()
		{
			auto const new_capacity = std::max(std::size_t{1}, 2*static_cast<size_t>(m_capacity));
			if(new_capacity > static_cast<size_t>(std::numeric_limits<size_type>::max()))
			{ throw std::runtime_error{"Data storage out of space"}; }

			realloc(new_capacity);
		}

		void realloc(size_type new_capacity)
		{
			auto const new_storage = std::tuple{std::make_unique_for_overwrite<Types[]>(new_capacity)...};
			if(m_size != 0)
			{ array_tuple_detail::move_data(m_storage, new_storage, m_size); }
			m_capacity = static_cast<size_type>(new_capacity);
			m_storage = std::move(new_storage);
		}

		size_type m_size;
		size_type m_capacity;
		std::tuple<std::unique_ptr<Types[]>...> m_storage;
	};
}

#endif