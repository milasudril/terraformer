#ifndef TERRAFORMER_ARRAY_TUPLE_HPP
#define TERRAFORMER_ARRAY_TUPLE_HPP

#include "./tuple.hpp"
#include "./utils.hpp"

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
		template <size_t... Is, class TupleLHS, class TupleRHS>
		void assign(std::index_sequence<Is...>, TupleLHS const& l, size_t index, TupleRHS&& r)
		{
			using tuple_rhs = std::remove_cvref_t<TupleRHS>;
			(...,(
			get<Is>(l)[index] = std::forward<std::tuple_element_t<Is, tuple_rhs>>(get<Is>(r))
			));
		}

		template <size_t... Is, class TupleLHS, class TupleRHS>
		void construct_at(std::index_sequence<Is...>, TupleLHS const& l, size_t index, TupleRHS&& r)
		{
			using tuple_rhs = std::remove_cvref_t<TupleRHS>;
			(...,(
				std::construct_at(get<Is>(l) + index,
					std::forward<std::tuple_element_t<Is, tuple_rhs>>(get<Is>(r)))
			));
		}

		template <size_t... Is, class Tuple>
		void destroy(std::index_sequence<Is...>, Tuple const& from, size_t size, size_t offset = 0)
		{
			(...,(
			std::destroy(get<Is>(from) + offset, get<Is>(from) + size)
			));
		}

		template <size_t... Is, class TupleFrom, class TupleTo>
		void uninitialized_move(std::index_sequence<Is...>, TupleFrom const& from, TupleTo const& to, size_t size)
		{
			(...,(
			std::uninitialized_move(get<Is>(from), get<Is>(from) + size, get<Is>(to))
			));
		}

		template <size_t... Is, class TupleFrom, class TupleTo>
		void uninitialized_copy(std::index_sequence<Is...>, TupleFrom const& from, TupleTo const& to, size_t size)
		{
			(...,(
			std::uninitialized_copy(get<Is>(from), get<Is>(from) + size, get<Is>(to))
			));
		}

		template <size_t... Is, class Tuple>
		bool equal(std::index_sequence<Is...>, Tuple const& a, Tuple const& b, size_t size)
		{
			return (... &&
			std::equal(get<Is>(a), get<Is>(a) + size, get<Is>(b))
			);
		}

		template<class T>
		class ptr_to_buffer_holder
		{
		public:
			using type = T;

			explicit ptr_to_buffer_holder(void* ptr):m_ptr{ptr}{}

			auto get() const
			{ return m_ptr; }

		private:
			void* m_ptr;
		};

		static_assert(sizeof(ptr_to_buffer_holder<int>) == sizeof(void*));

		template<class ... Types>
		using temp_storage = tuple<ptr_to_buffer_holder<Types>...>;

		template<class ... Types>
		auto make_temp_storage(size_t elem_count)
		{
			return temp_storage
			{
				// Do not care about the possibility of nullptr for now. Let it crash if
				// a pointer is nullptr
				// FIXME: Potential arithmetic overflow!
				ptr_to_buffer_holder<Types>{malloc(elem_count*sizeof(Types))}...
			};
		}
	}

	template<class ... Types>
	class [[deprecated("Use multi_array")]] array_tuple
	{
		using storage_type = tuple<Types*...>;
	public:
		using size_type = size_t;
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
					return tuple{as_ptr_to_const(item)...};
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

		array_tuple():m_size{0},m_capacity{0},m_storage{}
		{}

		explicit array_tuple(size_type n):m_size{n}, m_capacity{n}, m_storage{create_storage(n)}
		{
			for(size_type k = 0; k != n; ++k)
			{
				array_tuple_detail::construct_at(
					std::make_index_sequence<sizeof...(Types)>{},
					m_storage,
					k,
					std::tuple<Types...>{}
				);
			}
		}

		~array_tuple()
		{ free(); }

		[[nodiscard]] array_tuple(array_tuple&& other) noexcept:
			m_size{other.m_size},
			m_capacity{other.m_capacity},
			m_storage{other.m_storage}
		{
			other.m_size = 0;
			other.m_capacity = 0;
			other.m_storage = storage_type{};
		}

		[[nodiscard]] array_tuple(array_tuple const& other):
			m_size{other.m_size},
			m_capacity{other.m_capacity},
			m_storage{create_storage(other.m_capacity)}
		{
			array_tuple_detail::uninitialized_copy(std::make_index_sequence<sizeof...(Types)>{},
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
			free();
			m_size = std::exchange(other.m_size, 0);
			m_capacity = std::exchange(other.m_capacity, 0);
			m_storage = std::exchange(other.m_storage, storage_type{});
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
			return m_size == other.m_size &&
			array_tuple_detail::equal(std::make_index_sequence<sizeof...(Types)>{},
				other.m_storage,
				m_storage,
				m_size);
		}

		[[nodiscard]] bool operator!=(array_tuple const& other) const
		{ return !(*this == other); }

		template<class Tuple>
		requires(same_as_unqual<Tuple, value_type>)
		void push_back(Tuple&& t)
		{
			if(m_size == m_capacity) [[unlikely]]
			{realloc();}

			array_tuple_detail::construct_at(std::make_index_sequence<sizeof...(Types)>{},
				m_storage, m_size, std::forward<Tuple>(t));
			++m_size;
		}

		[[nodiscard]] bool empty() const
		{ return size() == 0; }

		[[nodiscard]] size_type size() const
		{ return m_size; }

		[[nodiscard]] size_type capacity() const
		{ return m_capacity; }

		void shrink(size_t new_size)
		{
			if(new_size == m_size) { return; }

			assert(new_size < size());

			array_tuple_detail::destroy(
				std::make_index_sequence<sizeof...(Types)>{},
				m_storage,
				m_size,
				new_size
			);
			m_size = new_size;
		}



		const_iterator begin() const
		{ return const_iterator{0, m_storage}; }

		const_iterator end() const
		{ return const_iterator{m_size, m_storage}; }

		template<class Dummy = void>
		requires(std::is_trivially_copyable_v<value_type>)
		[[nodiscard]] value_type operator[](size_type index) const
		{
			assert(index < m_size);
			return terraformer::apply([index](auto const& ... items){
				return value_type{items[index]...};
			}, m_storage);
		}

		template<class Dummy = void>
		requires(!std::is_trivially_copyable_v<value_type>)
		[[nodiscard]] cref_value_type operator[](size_type index) const
		{
			assert(index < m_size);
			return terraformer::apply([index](auto const& ... items){
				return cref_value_type{items[index]...};
			}, m_storage);
		}



		template<size_t I>
		[[nodiscard]] auto get()
		{ return std::span{terraformer::get<I>(m_storage), m_size}; }

		template<size_t I>
		[[nodiscard]] auto get() const
		{
			return std::span{as_ptr_to_const(terraformer::get<I>(m_storage)), m_size};
		}



		template<class Tuple>
		requires(same_as_unqual<Tuple, value_type>)
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

		static auto create_storage(size_type capacity)
		{
			auto new_buffer = array_tuple_detail::make_temp_storage<Types...>(capacity);
			auto new_storage = terraformer::apply([](auto ... item){
				return tuple{reinterpret_cast<std::decay_t<decltype(item)>::type*>(item.get())...};
			}, new_buffer);
			return new_storage;
		}

		void realloc(size_type new_capacity)
		{
			auto new_storage = create_storage(new_capacity);

			if(m_size != 0)
			{
				array_tuple_detail::uninitialized_move(std::make_index_sequence<sizeof...(Types)>{},
					m_storage, new_storage, m_size);
			}
			m_capacity = static_cast<size_type>(new_capacity);
			terraformer::apply([](auto... item){
				(..., ::free(item));
			}, m_storage);
			m_storage = new_storage;
		}

		void free()
		{
			array_tuple_detail::destroy(std::make_index_sequence<sizeof...(Types)>{},
				m_storage,
				m_size);

			terraformer::apply([](auto... item){
				(..., ::free(item));
			}, m_storage);
		}

		size_type m_size;
		size_type m_capacity;
		storage_type m_storage;
	};

	template<std::size_t I, class ... Types>
	[[nodiscard]] decltype(auto) get(array_tuple<Types...>& t)
	{ return t.template get<I>(); }

	template<std::size_t I, class ... Types>
	[[nodiscard]] decltype(auto) get(array_tuple<Types...> const& t)
	{ return t.template get<I>(); }
}

#endif