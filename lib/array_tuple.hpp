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
		void do_assign(std::index_sequence<Is...>, TupleLHS& l, size_t index, TupleRHS&& r)
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
	}

	template<class ... Types>
	class array_tuple
	{
	public:
		using size_type = uint32_t;

		template<class ... Vals>
		requires(array_tuple_detail::same_as_unqual<Types, Vals> && ...)
		void push_back(Vals&&... vals)
		{
			if(m_size == m_capacity) [[unlikely]]
			{realloc();}

			array_tuple_detail::do_assign(std::make_index_sequence<sizeof...(Types)>{},
				m_storage, m_size, std::tuple{std::forward<Vals>(vals)...});
			++m_size;
		}

		size_type size() const
		{ return m_size; }

		size_type capacity() const
		{ return m_capacity; }

		template<size_t I>
		auto get()
		{ return std::span{std::get<I>(m_storage).get(), m_size}; }

		template<size_t I>
		auto get() const
		{ return std::span{std::as_const(std::get<I>(m_storage).get()), m_size}; }

		auto get(size_type index) const
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
			array_tuple_detail::do_assign(std::make_index_sequence<sizeof...(Types)>{},
				m_storage, index, std::tuple{std::forward<Vals>(vals)...});
		}

	private:
		void realloc()
		{
			auto const new_capacity = std::max(std::size_t{1}, 2*static_cast<size_t>(m_capacity));
			if(new_capacity > static_cast<size_t>(std::numeric_limits<size_type>::max()))
			{ throw std::runtime_error{"Data storage out of space"}; }

			auto const new_storage = std::tuple{std::make_unique_for_overwrite<Types[]>(new_capacity)...};
			array_tuple_detail::move_data(m_storage, new_storage);
			m_capacity = static_cast<size_type>(new_capacity);
			m_storage = std::move(new_storage);
		}

		size_type m_size{0};
		size_type m_capacity{0};
		std::tuple<std::unique_ptr<Types[]>...> m_storage;
	};
}

#endif