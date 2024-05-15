#ifndef TERRAFORMER_FLATMAP_HPP
#define TERRAFORMER_FLATMAP_HPP

#include "lib/array_classes/multi_array.hpp"

namespace terraformer
{
	template<class Compare, class Key, class ... Value>
	class flat_map
	{
	public:
		using storage_type = multi_array<Key, Value...>;
		using index_type = storage_type::index_type;
		static constexpr index_type npos{static_cast<size_t>(-1)};

		template<class KeyType, class ... ValueType>
		requires std::is_same_v<
				multi_array_tag<std::remove_cvref_t<KeyType>, std::remove_cvref_t<ValueType>...>,
				multi_array_tag<Key, Value...>
			>
		|| requires (KeyType const& key, ValueType const&... values)
		{
			{multi_array_tag<Key, Value...>::convert(key, values...)};
		}
		[[nodiscard]] std::pair<index_type, bool> insert(KeyType&& key, ValueType&&... values)
		{
			auto const key_array = keys();
			auto const i = std::ranges::lower_bound(key_array, std::as_const(key), std::as_const(m_compare));
			index_type index{static_cast<size_t>(i - std::begin(key_array))};
			if(i == std::end(key_array) || m_compare(*i, key) || m_compare(key, *i))
			{
				m_storage.insert(index, std::forward<KeyType>(key), std::forward<ValueType>(values)...);
				return std::pair{index, true};
			}
			return std::pair{index, false};
		}

		template<class KeyType, class ... ValueType>
		requires std::is_same_v<
				multi_array_tag<std::remove_cvref_t<KeyType>, std::remove_cvref_t<ValueType>...>,
				multi_array_tag<Key, Value...>
			>
		|| requires (KeyType const& key, ValueType const&... values)
		{
			{multi_array_tag<Key, Value...>::convert(key, values...)};
		}
		[[nodiscard]] std::pair<index_type, bool> insert_or_assign(KeyType&& key, ValueType&&... values)
		{
			auto const key_array = keys();
			auto const i = std::ranges::lower_bound(key_array, std::as_const(key), std::as_const(m_compare));
			index_type index{static_cast<size_t>(i - std::begin(key_array))};
			if(i == std::end(key_array) || m_compare(*i, key) || m_compare(key, *i))
			{
				m_storage.insert(index, std::forward<KeyType>(key), std::forward<ValueType>(values)...);
				return std::pair{index, true};
			}
			m_storage.template assign<1>(index, std::forward<ValueType>(values)...);
			return std::pair{index, false};
		}

		template<class KeyType>
		[[nodiscard]] index_type find(KeyType&& key) const
		{
			auto const key_array = keys();
			auto const i = std::lower_bound(std::begin(key_array), std::end(key_array), key, m_compare);
			if(i == std::end(key_array) || m_compare(*i, key) || m_compare(key, *i))
			{ return npos; }
			return index_type{static_cast<size_t>(i - std::begin(key_array))};
		}

		[[nodiscard]] auto keys() const
		{ return m_storage.template get<0>(); }

		template<size_t Index>
		[[nodiscard]] auto values() const
		{ return m_storage.template get<Index + 1>(); }

		template<size_t Index>
		[[nodiscard]] auto values()
		{ return m_storage.template get<Index + 1>(); }

		void reserve(size_t capacity)
		{ m_storage.reserve(typename storage_type::size_type{capacity}); }

	private:
		[[no_unique_address]] Compare m_compare;
		multi_array<Key, Value...> m_storage;
	};
};

#endif