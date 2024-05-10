#ifndef TERRAFORMER_FLATMAP_HPP
#define TERRAFORMER_FLATMAP_HPP

#include "lib/array_classes/multi_array.hpp"

#include <functional>

namespace terraformer
{
	template<class Key, class Value, class Compare = std::less<Key>>
	class flat_map
	{
	public:
		using storage_type = multi_array<Key, Value>;
		using index_type = storage_type::index_type;
		static constexpr index_type npos{static_cast<size_t>(-1)};

		using key_type = Key;
		using mapped_type = Value;
		using value_type = std::pair<key_type, mapped_type>;
		using reference = std::pair<key_type const&, mapped_type&>;
		using const_reference = std::pair<key_type const&, mapped_type const&>;

		std::pair<index_type, bool> insert(value_type&& value)
		{
			auto const key_array = keys();
			auto const i = std::ranges::lower_bound(key_array, value.first, std::as_const(m_compare));
			if(i == std::end(key_array) || m_compare(*i, value.first) || m_compare(value.first, *i))
			{
				auto const index = i - std::begin(key_array);
				m_storage.insert(index, std::move(value.first), std::move(value.second));
				return std::pair{index_type{index}, true};
			}
			return std::pair{index_type{i - std::begin(key_array)}, false};
		}

		index_type find(key_type const& key) const
		{
			auto const i = std::ranges::lower_bound(key_array, value.first, std::as_const(m_compare));
			if(i == std::end(key_array) || m_compare(*i, value.first) || m_compare(value.first, *i))
			{ return npos; }
			return std::pair{index_type{i - std::begin(key_array)}, false};
		}

		auto keys() const
		{ return m_storage.get<0>(); }

		auto values() const
		{ return m_storage.get<1>(); }

	private:
		[[no_unique_address]] Compare m_compare;
		multi_array<Key, Value> m_storage;
	};
};

#endif