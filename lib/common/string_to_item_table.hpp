#ifndef TERRAFORMER_STRING_TO_ITEM_TABLE_HPP
#define TERRAFORMER_STRING_TO_ITEM_TABLE_HPP

#include "./flat_map.hpp"
#include <cstring>

namespace terraformer
{
	constexpr auto make_hash(char const* str)
	{
		auto hash = 0xcbf29ce484222325llu;
		auto const fnv_prime = 0x100000001b3llu;
		while(*str != '\0')
		{
			hash ^= static_cast<uint8_t>(*str);
			hash *= fnv_prime;
			++str;
		}
		return hash;
	};

	template<class StringStorage>
	struct string_compare_less
	{
		bool operator()(StringStorage const& a, StringStorage const& b) const
		{ return strcmp(a.get(), b.get()) < 0; }

		bool operator()(char const* a, StringStorage const& b) const
		{ return strcmp(a, b.get()) < 0; }

		bool operator()(StringStorage const& a, char const* b) const
		{ return strcmp(a.get(), b) < 0; }
	};

	template<class ItemType, bool direct = false>
	class string_to_item_table;

	template<class ItemType>
	class string_to_item_table<ItemType, true>
	{
	public:
		using stored_key = std::unique_ptr<char[]>;

		static auto make_stored_key(char const* key)
		{
			auto length = strlen(key);
			auto ret = std::make_unique<char[]>(length + 1);
			memcpy(ret.get(), key, length + 1);
			return ret;
		}

		template<class Item>
		explicit string_to_item_table(char const* first_key, Item&& first_item):
			m_first_key_value{first_key, std::forward<Item>(first_item)}
		{}

		ItemType const* at_ptr(char const* key) const
		{
			if(m_first_key_value.key == key) [[likely]]
			{ return &m_first_key_value.value; }

			auto const i = m_other_items.find(key);
			if(i == flat_map<string_compare_less<stored_key>, stored_key, ItemType>::npos)
			{ return nullptr; }

			return &m_other_items.template values<0>()[i];
		}

		auto const& first_element_value() const
		{ return m_first_key_value.value; }

		auto& first_element_value()
		{ return m_first_key_value.value; }

		template<class Item>
		auto insert(char const* key, Item&& value)
		{
			if(key == m_first_key_value.key) [[unlikely]]
			{ return std::pair{&m_first_key_value.value, false}; }

			auto const i = m_other_items.insert(make_stored_key(key), std::forward<Item>(value));
			return std::pair{&m_other_items.template values<0>()[i.first], i.second};
		}

		template<class Item>
		auto insert_or_assign(char const* key, Item&& value)
		{
			if(key == m_first_key_value.key) [[unlikely]]
			{
				m_first_key_value.value = std::forward<Item>(value);
				return std::pair{&m_first_key_value.value, false};
			}

			auto const i = m_other_items.insert_or_assign(make_stored_key(key), std::forward<Item>(value));
			return std::pair{&m_other_items.template values<0>()[i.first], i.second};
		}

	private:
		struct match
		{
			std::string key;
			ItemType value;
		};

		match m_first_key_value;
		flat_map<string_compare_less<stored_key>, stored_key, ItemType> m_other_items;
	};

	template<class ItemType>
	class string_to_item_table<ItemType, false>
	{
	public:
		using storage_type = flat_map<std::less<>, uint64_t, string_to_item_table<ItemType, true>>;

		ItemType const* at_ptr(char const* key) const
		{
			auto const hashed_key = make_hash(key);
			auto const i = m_storage.find(hashed_key);
			if(i == storage_type::npos) [[unlikely]]
			{ return nullptr; }

			return m_storage.template values<0>()[i].at_ptr(key);
		}

		template<class Item>
		auto insert(char const* key, Item&& value)
		{
			auto const hashed_key = make_hash(key);
			auto const i = m_storage.find(hashed_key);
			if(i == storage_type::npos) [[likely]]
			{
				[[maybe_unused]] auto const insert_res = m_storage.insert(
					hashed_key,
					string_to_item_table<ItemType, true>{
						key,
						std::forward<Item>(value)
					}
				);

				return std::pair{&m_storage.template values<0>()[i].first_element_value(), true};
			}

			return m_storage.template values<0>()[i].insert(key, std::move(value));
		}

		template<class Item>
		auto insert_or_assign(char const* key, Item&& value)
		{
			auto const hashed_key = make_hash(key);
			auto const i = m_storage.find(hashed_key);
			if(i == storage_type::npos) [[likely]]
			{
				[[maybe_unused]] auto const insert_res =  m_storage.insert(
					hashed_key,
					string_to_item_table<ItemType, true>{
						key,
						std::forward<Item>(value)
					}
				);

				return std::pair{&m_storage.template values<0>()[i].first_element_value(), true};
			}
			return m_storage.template values<0>()[i].insert_or_assign(key, std::forward<Item>(value));
		}

	private:
		storage_type m_storage;
	};
}

#endif