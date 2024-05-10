#ifndef TERRAFORMER_FLATMAP_HPP
#define TERRAFORMER_FLATMAP_HPP

#include "lib/array_classes/multi_array.hpp"

#include <functional>

namespace terraformer
{
	template<class Key, class ... Value, class Compare = std::less<Key>>
	class flat_map
	{
	public:
		using storage_type = multi_array<Key, Value...>;
		using index_type = storage_type::index_type;
		static constexpr index_type npos{static_cast<size_t>(-1)};


		template<class KeyType, class ... ValueType>
		requires
			std::is_same_v<
				multi_array_tag<std::remove_cvref_t<KeyType>, std::remove_cvref_t<ValueType>...>,
				multi_array_tag<Key, Value...>
			>
		|| requires (KeyType const& key, ValueType const&... values)
		{
			{multi_array_tag<Key, Value...>::convert(key, values...)};
		}
		std::pair<index_type, bool> insert(KeyType&& key, ValueType&&... values)
		{
			auto const key_array = keys();
			auto const i = std::ranges::lower_bound(key_array, std::as_const(key), std::as_const(m_compare));
			if(i == std::end(key_array) || m_compare(*i, value.first) || m_compare(value.first, *i))
			{
				auto const index = i - std::begin(key_array);
				m_storage.insert(index, std::forward<KeyType>(key), std::forward<KeyType>(values)...);
				return std::pair{index_type{index}, true};
			}
			return std::pair{index_type{i - std::begin(key_array)}, false};
		}

		template<class KeyType>
		requires std::is_same_v<std::remove_cvref_t<KeyType>>, Key>
		index_type find(KeyType&& key) const
		{
			auto const i = std::ranges::lower_bound(key_array, key, std::as_const(m_compare));
			if(i == std::end(key_array) || m_compare(*i, key) || m_compare(key, *i))
			{ return npos; }
			return std::pair{index_type{i - std::begin(key_array)}, false};
		}

		auto keys() const
		{ return m_storage.get<0>(); }

		template<size_t Index>
		auto values() const
		{ return m_storage.get<Index - 1>(); }

		template<size_t Index>
		auto values()
		{ return m_storage.get<Index - 1>(); }

	private:
		[[no_unique_address]] Compare m_compare;
		multi_array<Key, Value...> m_storage;
	};
};

#endif