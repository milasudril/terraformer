#ifndef TERRAFORMER_SINGLE_ARRAY_HPP
#define TERRAFORMER_SINGLE_ARRAY_HPP

#include "./memory_block.hpp"
#include "./span.hpp"

namespace terraformer
{
	template<class T>
	class single_array
	{
	public:
		using size_type = array_size<T>;
		using index_type = array_index<T>;

		single_array() noexcept= default;

		explicit single_array(size_type size)
		{ resize(size); }

		single_array(single_array&& other) noexcept:
			m_storage{std::exchange(other.m_storage, memory_block{})},
			m_size{std::exchange(other.m_size, size_type{})},
			m_capacity{std::exchange(other.m_capacity, size_type{})}
		{ }

		single_array(single_array const& other):
			m_storage{make_byte_size(other.capacity())},
			m_size{other.m_size},
			m_capacity{other.m_capacity}
		{ std::uninitialized_copy_n(other.begin(), m_size.get(), m_storage.template interpret_as<T>()); }

		single_array& operator=(single_array&& other) noexcept
		{
			clear();
			m_storage = std::exchange(other.m_storage, memory_block{});
			m_size = std::exchange(other.m_size, size_type{});
			m_capacity = std::exchange(other.m_capacity, size_type{});
			return *this;
		}

		single_array& operator=(single_array const& other) = delete;

		~single_array() noexcept
		{ clear(); }

		constexpr auto first_element_index() const noexcept
		{ return index_type{}; }

		constexpr auto last_element_index() const noexcept
		{ return index_type{(m_size - size_type{1}).get()}; }

		auto size() const noexcept
		{ return m_size; }

		auto empty() const noexcept
		{ return m_size.get() == 0; }

		auto capacity() const noexcept
		{ return m_capacity; }

		auto data() noexcept
		{ return m_storage.interpret_as<T>(); }

		auto data() const noexcept
		{ return m_storage.interpret_as<T const>(); }

		auto begin() noexcept
		{ return data(); }

		auto begin() const noexcept
		{ return data(); }

		auto end() noexcept
		{ return begin() + size().get(); }

		auto end() const noexcept
		{ return begin() + size().get(); }

		void reserve(size_type new_capacity)
		{
			if(new_capacity > m_capacity)
			{
				memory_block new_storage{make_byte_size(new_capacity)};
				std::uninitialized_move(begin(), end(), new_storage.interpret_as<T>());
				std::destroy(begin(), end());
				m_storage = std::move(new_storage);
				m_capacity = new_capacity;
			}
		}

		template<class Arg>
		requires (std::is_same_v<std::remove_cvref_t<Arg>, T> || std::is_convertible_v<Arg, T>)
		void push_back(Arg&& elem)
		{
			auto new_size = m_size + size_type{1};
			if(new_size > m_capacity)
			{ reserve(std::max(size_type{8}, static_cast<size_t>(2)*capacity())); }
			std::construct_at(m_storage.interpret_as<T>() + m_size.get(), std::forward<Arg>(elem));
			m_size = new_size;
		}

		void clear() noexcept
		{
			std::destroy(begin(), end());
			m_size = size_type{};
		}

		void resize(size_type new_size)
		{
			if(new_size < m_size)
			{
				truncate_from(index_type{new_size.get()});
				return;
			}

			if(new_size > m_size)
			{
				if(new_size > m_capacity)
				{ reserve(new_size); }
				std::uninitialized_default_construct_n(end(), (new_size - m_size).get());
				m_size = new_size;
				return;
			}
		}

		void truncate_from(index_type index) noexcept
		{
			std::destroy(begin() + index.get(), end());
			m_size = size_type{index};
		}

		auto& operator[](index_type index) noexcept
		{ return deref(data(), index); }

		auto& operator[](index_type index) const noexcept
		{ return deref(data(), index); }

		operator span<T, index_type, size_type>() noexcept
		{ return span<T, index_type, size_type>{begin(), end()}; }

		operator span<T const>() const noexcept
		{ return span<T const, index_type, size_type>{begin(), end()}; }

	private:
		memory_block m_storage{};
		size_type m_size{};
 		size_type m_capacity{};
	};
}

#endif
