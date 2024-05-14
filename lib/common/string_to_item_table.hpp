#ifndef TERRAFORMER_STRING_TO_ITEM_TABLE_HPP
#define TERRAFORMER_STRING_TO_ITEM_TABLE_HPP

#include "./flat_map.hpp"

namespace terraformer
{
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

		ItemType const* at_ptr(char const* key) const
		{
			if(!m_first_item.has_value())
			{ return nullptr; }

			if(m_first_item.key == key)
			{ return &m_first_item->value; }

			return m_other_items.at_ptr(key);
		}

	private:
		struct match
		{
			std::string key;
			ItemType value;
		};

		std::optional<match> m_first_item;
		flat_map<string_compare_less<stored_key>, stored_key, ItemType> m_other_items;
	};

	template<class ItemType>
	class string_to_item_table<ItemType, false>
	{
	public:
		using storage_type = flat_map<std::less<>, uint64_t, string_to_item_table<ItemType, true>>;

		ItemType const* at_ptr(char const* key) const
		{
			auto const ptr = m_storage.at_ptr(make_hash(key));
			if(ptr == nullptr)
			{ return nullptr; }

			return ptr->at_ptr(key);
		}

	private:
		static constexpr auto make_hash(char const* str)
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

		storage_type m_storage;
	};
}

#endif