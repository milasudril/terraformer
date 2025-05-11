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
		using value_type = T;
		static constexpr index_type npos{static_cast<size_t>(-1)};

		single_array() noexcept= default;

		explicit single_array(size_type size)
		{ resize(size); }

		single_array(span<value_type const> vals):
			m_storage{make_byte_size<T>(std::size(vals))},
			m_size{std::size(vals)},
			m_capacity{std::size(vals)}
		{ std::uninitialized_copy_n(vals.begin(), m_size.get(), m_storage.template interpret_as<T>()); }

		single_array(single_array&& other) noexcept:
			m_storage{std::exchange(other.m_storage, memory_block{})},
			m_size{std::exchange(other.m_size, size_type{})},
			m_capacity{std::exchange(other.m_capacity, size_type{})}
		{ }

		single_array(single_array const& other):
			m_storage{make_byte_size<T>(other.capacity())},
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

		auto element_indices(size_t skip = 0) const noexcept
		{ return index_range{index_type{} + skip, index_type{} + m_size.get()}; }

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
				memory_block new_storage{make_byte_size<T>(new_capacity)};
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


		template<class Arg>
		requires (std::is_same_v<std::remove_cvref_t<Arg>, T> || std::is_convertible_v<Arg, T>)
		void insert_or_assign(index_type index, Arg&& elem)
		{
			if(size_type{index} >= size()) [[unlikely]]
			{ resize(size_type{index} + size_type{1}); }
			(*this)[index] = std::forward<Arg>(elem);
		}

		void pop_back()
		{ truncate_from(index_type{(m_size - size_type{1}).get()}); }

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
				{ reserve(std::max(new_size, static_cast<size_t>(2)*m_capacity)); }
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

		template<class U>
		T value_or(index_type index, U&& default_value) const noexcept
		{
			if(index < m_size)
			{ return (*this)[index]; }

			return T{std::forward<U>(default_value)};
		}

		operator span<T, index_type, size_type>() noexcept
		{ return span<T, index_type, size_type>{begin(), end()}; }

		operator span<T const>() const noexcept
		{ return span<T const, index_type, size_type>{begin(), end()}; }

		auto& front() noexcept
		{ return *begin(); }

		auto const& front() const noexcept
		{ return *begin(); }

		auto& back() noexcept
		{ return *(end() - 1); }

		auto const& back() const noexcept
		{ return *(end() - 1);}

	private:
		memory_block m_storage{};
		size_type m_size{};
 		size_type m_capacity{};
	};
}

#endif
