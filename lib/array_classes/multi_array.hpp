#ifndef TERRAFORMER_MULTI_ARRAY_HPP
#define TERRAFORMER_MULTI_ARRAY_HPP

#include "./memory_block.hpp"
#include "./multi_span.hpp"

#include <type_traits>
#include <cassert>

namespace terraformer
{
	template<size_t Index, class T>
	struct type_at{};

	template<size_t Index, class Head, class ...T>
	struct type_at<Index, multi_array_tag<Head, T...>>:type_at<Index - 1, multi_array_tag<T...>>
	{};

	template<class Head, class ...T>
	struct type_at<0, multi_array_tag<Head, T...>>
	{
		using type = Head;
	};

	template<size_t Index, class ArrayTag>
	using type_at_t = typename type_at<Index, ArrayTag>::type;

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
	void move_element(
		std::array<memory_block, sizeof...(T)> const& storage,
		array_index<multi_array_tag<T...>> from,
		array_index<multi_array_tag<T...>> to
	)
	{
		size_t index = 0;
		(
			(
				*(storage[index].template interpret_as<T>() + to.get())
					= std::move(*(storage[index].template interpret_as<T>() + from.get())),
				++index
			),...
		);
	}

	template<class ... T>
	void move_element_to_uninitialized(
		std::array<memory_block, sizeof...(T)> const& storage,
		array_index<multi_array_tag<T...>> from,
		array_index<multi_array_tag<T...>> to
	)
	{
		size_t index = 0;
		(
			(
				std::construct_at(
					storage[index].template interpret_as<T>() + to.get(),
					std::move(*(storage[index].template interpret_as<T>() + from.get()))
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

	template<size_t StartAt, class ArrayTag, class Head, class ... Args>
	void assign_values(
		std::array<memory_block, ArrayTag::type_count> const& dest,
		array_index<ArrayTag> offset,
		Head&& head,
		Args&&... args
	)
	{
		if constexpr(StartAt != ArrayTag::type_count)
		{
			using current_type = type_at_t<StartAt, ArrayTag>;
			auto obj = dest[StartAt].template interpret_as<current_type>() + offset.get();
			*obj = std::forward<Head>(head);
			if constexpr(sizeof...(args) != 0)
			{ assign_values<StartAt + 1>(dest, offset, std::forward<Args>(args)...); }
		}
	}

	template<class ... T>
	class multi_array
	{
	public:
		using storage_type = std::array<memory_block, sizeof...(T)>;
		using size_type = array_size<multi_array_tag<T...>>;
		using index_type = array_index<multi_array_tag<T...>>;
		using reference = tuple<T&...>;
		using const_reference = tuple<T const&...>;

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

		template<size_t From, class ... Arg>
		void assign(index_type index, Arg&&... args)
		{ assign_values<From>(m_storage, index, std::forward<Arg>(args)...); }


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

		auto attributes() noexcept
		{
			return multi_span<T...>{
				std::apply(
					[](auto const&... args){
						return tuple{args.template interpret_as<T>()...};
					},
					m_storage
				),
				m_size
			};
		}

		auto attributes() const noexcept
		{
			return multi_span<T const...>{
				std::apply(
					[](auto const&... args){
						return tuple{args.template interpret_as<T const>()...};
					},
					m_storage
				),
				m_size
			};
		}

		void truncate_from(index_type index) noexcept
		{
			destroy(m_storage, index, size_type{m_size.get() - index.get()});
			m_size = size_type{index};
		}

		reference operator[](index_type index) noexcept
		{
			assert(index < m_size);
			return std::apply(
				[index](auto const&... args) {
					return tuple<T&...>{*(args.template interpret_as<T>() + index.get())...};
				},
				m_storage
			);
		}

		const_reference operator[](index_type index) const noexcept
		{
			assert(index < m_size);

			return std::apply(
				[index](auto const&... args) {
					return tuple<T const&...>{*(args.template interpret_as<T const>() + index.get())...};
				},
				m_storage
			);
		}


		template<class ... Arg>
		requires std::is_same_v<multi_array_tag<std::remove_cvref_t<Arg>...>, multi_array_tag<T...>>
		|| requires (Arg const&... args)
		{
			{multi_array_tag<T...>::convert(args...)};
		}
		void insert(index_type index, Arg&&... elems)
		{
			auto new_size = m_size + size_type{1};
			if(new_size > m_capacity)
			{ reserve(std::max(size_type{8}, static_cast<size_t>(2)*capacity())); }

			if(index.get() >= m_size.get())
			{
				construct(m_storage, index_type{m_size.get()}, std::forward<Arg>(elems)...);
				m_size = new_size;
				return;
			}
			assert(m_size.get() != 0);

			auto k = index_type{m_size.get() - 1};
			move_element_to_uninitialized(m_storage, k, index_type{m_size.get()});
			while(k != index)
			{
				move_element(m_storage, k - 1, k);
				--k;
			}
			destroy(m_storage, index, size_type{1});
			construct(m_storage, index, std::forward<Arg>(elems)...);
			m_size = new_size;
		}

	private:
		storage_type m_storage{};
		size_type m_size{};
 		size_type m_capacity{};
	};

	template<class>
	struct compatible_multi_array;

	template<class ... T>
	struct compatible_multi_array<multi_span<T...>>
	{
		using type = multi_array<std::remove_const_t<T>...>;
	};

	template<class ... T>
	using compatible_multi_array_t = compatible_multi_array<T...>::type;
}

#endif
