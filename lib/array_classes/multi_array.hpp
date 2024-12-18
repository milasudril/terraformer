#ifndef TERRAFORMER_MULTI_ARRAY_HPP
#define TERRAFORMER_MULTI_ARRAY_HPP

#include "./memory_block.hpp"
#include "./span.hpp"
#include "lib/common/tuple.hpp"

#include <type_traits>

namespace terraformer
{
	template<class ... T>
	struct multi_array_tag
	{
		template<class U>
		requires std::disjunction_v<std::is_same<U, T>...>
		static consteval auto match_tag()
		{ return std::type_identity<U>{}; }

		static consteval void convert(T const&...){}
	};

	template<class ... T>
	auto generate_mem_blocks(array_size<multi_array_tag<T...>> size)
	{
		return std::array<memory_block, sizeof...(T)>{
			memory_block{make_byte_size(array_size<T>{size})}...
		};
	}

	template<class ... T, class ... Args>
	void construct(
		std::array<memory_block, sizeof...(T)> const& storage,
		array_index<multi_array_tag<T...>> offset,
		Args&&... values
	)
	{
		size_t index = 0;
		(
			(
				std::construct_at(
					storage[index].template interpret_as<T>() + offset.get(),
					std::forward<Args>(values)
				),
				++index
			),...
		);
	}

	template<class ... T>
	void uninitialized_copy(
		std::array<memory_block, sizeof...(T)> const& src,
		std::array<memory_block, sizeof...(T)> const& dest,
		array_size<multi_array_tag<T...>> n
	)
	{
		size_t index = 0;
		(
			(
				std::uninitialized_copy_n(
					src[index].template interpret_as<T>(),
					n.get(),
					dest[index].template interpret_as<T>()
				),
				++index
			),...
		);
	}

	template<class ... T>
	void uninitialized_move(
		std::array<memory_block, sizeof...(T)> const& src,
		std::array<memory_block, sizeof...(T)> const& dest,
		array_size<multi_array_tag<T...>> n
	) noexcept
	{
		size_t index = 0;
		(
			(
				std::uninitialized_move_n(
					src[index].template interpret_as<T>(),
					n.get(),
					dest[index].template interpret_as<T>()
				),
				++index
			),...
		);
	}

	template<class ... T>
	void destroy(
		std::array<memory_block, sizeof...(T)> const& src,
		array_index<multi_array_tag<T...>> offset,
		array_size<multi_array_tag<T...>> n
	) noexcept
	{
		size_t index = 0;
		(
			(
				std::destroy_n(
					src[index].template interpret_as<T>() + offset.get(),
					n.get()
				),
				++index
			),...
		);
	}

	template<class ... T>
	void uninitialized_default_construct(
		std::array<memory_block, sizeof...(T)> const& dest,
		array_index<multi_array_tag<T...>> offset,
		array_size<multi_array_tag<T...>> n
	)
	{
		size_t index = 0;
		(
			(
				std::uninitialized_default_construct_n(
					dest[index].template interpret_as<T>() + offset.get(),
					n.get()
				),
				++index
			),...
		);
	}

	template<class ... T>
	class multi_array
	{
	public:
		using storage_type = std::array<memory_block, sizeof...(T)>;
		using size_type = array_size<multi_array_tag<T...>>;
		using index_type = array_index<multi_array_tag<T...>>;

		template<size_t Index>
		using attribute_type = std::tuple_element_t<Index, tuple<T...>>;

		multi_array() noexcept = default;

		explicit multi_array(size_type size) noexcept
		{ resize(size); }

		multi_array(multi_array&& other) noexcept:
			m_storage{std::exchange(other.m_storage, storage_type{})},
			m_size{std::exchange(other.m_size, size_type{})},
			m_capacity{std::exchange(other.m_capacity, size_type{})}
		{ }

		multi_array(multi_array const& other):
			m_storage{generate_mem_blocks(other.capacity())},
			m_size{other.m_size},
			m_capacity{other.m_capacity}
		{ uninitialized_copy(other.m_storage, m_storage, m_size); }

		multi_array& operator=(multi_array&& other) noexcept
		{
			clear();
			m_storage = std::exchange(other.m_storage, storage_type{});
			m_size = std::exchange(other.m_size, size_type{});
			m_capacity = std::exchange(other.m_capacity, size_type{});
			return *this;
		}

		multi_array& operator=(multi_array const& other) = delete;

		~multi_array() noexcept
		{ clear(); }

		auto element_indices(size_t skip = 0) const
		{ return index_range{index_type{} + skip, index_type{} + m_size.get()}; }

		auto size() const noexcept
		{ return m_size; }

		auto capacity() const noexcept
		{ return m_capacity; }

		auto empty() const noexcept
		{ return m_size.get() == 0; }

		void reserve(size_type new_capacity)
		{
			if(new_capacity > m_capacity)
			{
				auto new_storage = generate_mem_blocks(new_capacity);
				uninitialized_move(m_storage, new_storage, m_size);
				destroy(m_storage, index_type{}, m_size);
				m_storage = std::move(new_storage);
				m_capacity = new_capacity;
			}
		}

		template<class ... Arg>
		requires std::is_same_v<multi_array_tag<std::remove_cvref_t<Arg>...>, multi_array_tag<T...>>
		|| requires (Arg const&... args)
		{
			{multi_array_tag<T...>::convert(args...)};
		}
		void push_back(Arg&&... elems)
		{
			auto new_size = m_size + size_type{1};
			if(new_size > m_capacity)
			{ reserve(std::max(size_type{8}, static_cast<size_t>(2)*capacity())); }
			construct(m_storage, index_type{m_size.get()}, std::forward<Arg>(elems)...);
			m_size = new_size;
		}

		void clear() noexcept
		{
			destroy(m_storage, index_type{}, m_size);
			m_size = size_type{};
		}

		void resize(size_type new_size)
		{
			if(new_size < m_size)
			{
				truncate_from(index_type{new_size.get()});
				m_size = new_size;
				return;
			}

			if(new_size > m_size)
			{
				if(new_size > m_capacity)
				{ reserve(new_size); }

				uninitialized_default_construct(m_storage, index_type{m_size.get()}, new_size - m_size);
				m_size = new_size;
				return;
			}
		}

		template<size_t AttributeIndex>
		auto get() const noexcept
		{
			using sel_attribute_type = attribute_type<AttributeIndex> const;
			auto const ptr = m_storage[AttributeIndex].template interpret_as<sel_attribute_type>();
			return span<sel_attribute_type, index_type, size_type>{ptr, ptr + m_size.get()};
		}

		template<size_t AttributeIndex>
		auto get() noexcept
		{
			using sel_attribute_type = attribute_type<AttributeIndex>;
			auto const ptr = m_storage[AttributeIndex].template interpret_as<sel_attribute_type>();
			return span<sel_attribute_type, index_type, size_type>{ptr, ptr + m_size.get()};
		}

		void truncate_from(index_type index) noexcept
		{
			destroy(m_storage, index, size_type{m_size.get() - index.get()});
			m_size = size_type{index};
		}

	private:
		storage_type m_storage{};
		size_type m_size{};
 		size_type m_capacity{};
	};
}

#endif
