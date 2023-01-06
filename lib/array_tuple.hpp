#ifndef TERRAFORMER_LIB_ARRAY_TUPLE_HPP
#define TERRAFORMER_LIB_ARRAY_TUPLE_HPP

#include "./tuple.hpp"

#include <memory>
#include <algorithm>
#include <span>
#include <limits>
#include <concepts>
#include <cassert>
#include <string>

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
			get<Is>(l)[index] = std::forward<std::tuple_element_t<Is, TupleRHS>>(get<Is>(r))
			));
		}

		template <size_t... Is, class TupleFrom, class TupleTo>
		void move_data(std::index_sequence<Is...>, TupleFrom&& from, TupleTo& to, size_t size)
		{
			(...,(
			std::move(get<Is>(from).get(), get<Is>(from).get() + size, get<Is>(to).get())
			));
		}

		template <size_t... Is, class TupleFrom, class TupleTo>
		void copy_data(std::index_sequence<Is...>, TupleFrom const& from, TupleTo& to, size_t size)
		{
			(...,(
			std::copy(get<Is>(from).get(), get<Is>(from).get() + size, get<Is>(to).get())
			));
		}

		template <size_t... Is, class Tuple>
		bool equal(std::index_sequence<Is...>, Tuple const& a, Tuple const& b, size_t size)
		{
			return (... &&
			std::equal(get<Is>(a).get(), get<Is>(a).get() + size, get<Is>(b).get())
			);
		}
	}

	template<class ... Types>
	class array_tuple
	{
		using storage_type = tuple<std::unique_ptr<Types[]>...>;
	public:
		using size_type = uint32_t;
		using value_type = tuple<Types...>;
		using cref_value_type = tuple<Types const&...>;

		class const_iterator
		{
		public:
			using value_type = std::conditional_t<
				std::is_trivially_copyable_v<array_tuple::value_type>,
				array_tuple::value_type,
				array_tuple::cref_value_type>;

			const_iterator() = default;

			explicit const_iterator(size_t index, storage_type const& storage):
				m_index{index},
				m_base_pointers{terraformer::apply([](auto const& ... item){
					return tuple<Types const*...>{item.get()...};
				}, storage)}
			{}

			[[nodiscard]] value_type operator[](intptr_t n) const
			{
				return terraformer::apply([offset = m_index + n](auto ... items){
					return value_type{*(items + offset)...};
				}, m_base_pointers);
			}

			[[nodiscard]] value_type operator*() const
			{
				return terraformer::apply([offset = m_index](auto const& ... items){
					return value_type{*(items + offset)...};
				}, m_base_pointers);
			}

			auto current_offset() const
			{
				return m_index;
			}



			const_iterator& operator++()
			{
				++m_index;
				return *this;
			}

			const_iterator operator++(int)
			{
				auto old = *this;
				++m_index;
				return old;
			}

			const_iterator& operator--()
			{
				--m_index;
				return *this;
			}

			const_iterator operator--(int)
			{
				auto old = *this;
				--m_index;
				return old;
			}

			const_iterator& operator+=(intptr_t n)
			{
				m_index += n;
				return *this;
			}

			const_iterator& operator-=(intptr_t n)
			{
				m_index -= n;
				return *this;
			}

			[[nodiscard]] friend const_iterator operator+(const_iterator a, intptr_t n)
			{ return a += n; }

			[[nodiscard]] friend const_iterator operator+(intptr_t n, const_iterator a)
			{ return a += n; }

			[[nodiscard]] friend const_iterator operator-(const_iterator a, intptr_t n)
			{ return a -= n; }

			[[nodiscard]] friend intptr_t operator-(const_iterator const& a, const_iterator const& b)
			{ return a.m_index - b.m_index; }



			[[nodiscard]] bool operator==(const_iterator const& other) const
			{ return m_index == other.m_index; }

			[[nodiscard]] bool operator<(const_iterator const& other) const
			{ return m_index < other.m_index; }

			[[nodiscard]] bool operator<=(const_iterator const& other) const
			{ return m_index <= other.m_index; }

			[[nodiscard]] bool operator!=(const_iterator const& other) const
			{ return m_index != other.m_index; }

			[[nodiscard]] bool operator>(const_iterator const& other) const
			{ return m_index > other.m_index; }

			bool operator>=(const_iterator const& other) const
			{ return m_index >= other.m_index; }

		private:
			size_t m_index;
			tuple<Types const*...> m_base_pointers;
		};

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
			m_storage{tuple{std::make_unique_for_overwrite<Types[]>(other.capacity())...}}
		{
			array_tuple_detail::copy_data(std::make_index_sequence<sizeof...(Types)>{},
				other.m_storage,
				m_storage,
				m_size);
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
			array_tuple_detail::equal(std::make_index_sequence<sizeof...(Types)>{},
				other.m_storage,
				m_storage,
				m_size);
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
				m_storage, m_size, tuple{std::forward<Vals>(vals)...});
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



		const_iterator begin() const
		{ return const_iterator{0, m_storage}; }

		const_iterator end() const
		{ return const_iterator{m_size, m_storage}; }

		template<class Dummy = void>
		requires(std::is_trivially_copyable_v<value_type>)
		[[nodiscard]] value_type operator[](size_type index) const
		{
			assert(index < m_size);
			return apply([index](auto const& ... items){
				return value_type{items[index]...};
			}, m_storage);
		}

		template<class Dummy = void>
		requires(!std::is_trivially_copyable_v<value_type>)
		[[nodiscard]] cref_value_type operator[](size_type index) const
		{
			assert(index < m_size);

			return apply([index](auto const& ... items){
				return cref_value_type{items[index]...};
			}, m_storage);
		}



		template<size_t I>
		[[nodiscard]] auto get()
		{ return std::span{get<I>(m_storage).get(), m_size}; }

		template<size_t I>
		[[nodiscard]] auto get() const
		{ return std::span{std::as_const(get<I>(m_storage).get()), m_size}; }



		template<class ... Vals>
		requires(array_tuple_detail::same_as_unqual<Types, Vals> && ...)
		void assign(size_type index, Vals&&... vals)
		{
			assert(index < m_size);
			array_tuple_detail::assign(std::make_index_sequence<sizeof...(Types)>{},
				m_storage, index, tuple{std::forward<Vals>(vals)...});
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

			realloc(static_cast<size_type>(new_capacity));
		}

		void realloc(size_type new_capacity)
		{
			auto new_storage = tuple{std::make_unique_for_overwrite<Types[]>(new_capacity)...};
			if(m_size != 0)
			{
				array_tuple_detail::move_data(std::make_index_sequence<sizeof...(Types)>{},
					std::move(m_storage), new_storage, m_size);
			}
			m_capacity = static_cast<size_type>(new_capacity);
			m_storage = std::move(new_storage);
		}

		size_type m_size;
		size_type m_capacity;
		storage_type m_storage;
	};
}

#endif