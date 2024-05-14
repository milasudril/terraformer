#ifndef TERRAFORMER_STRING_TO_ITEM_TABLE_HPP
#define TERRAFORMER_STRING_TO_ITEM_TABLE_HPP

#include "./flat_map.hpp"

namespace terraformer
{
	template<class ItemType>
	class string_to_item_table
	{
	public:
	private:
		using stored_key = std::unique_ptr<char[]>;

		struct compare
		{
			bool operator()(stored_key const& a, stored_key const& b) const
			{ return strcmp(a.get(), b.get()) < 0; }

			bool operator()(char const* a, stored_key const& b) const
			{ return strcmp(a, b.get()) < 0; }

			bool operator()(stored_key const& a, char const* b) const
			{ return strcmp(a.get(), b) < 0; }
		};

		struct match
		{
			stored_key key;
			Item value;
		};

		struct collision_resolver
		{
			match first_match;
			flat_map<compare, stored_key, ItemType> other_matches;
		};

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

		flat_map<std::less<>, uint64_t, collision_resolver> m_storage;
	};
}

#endif