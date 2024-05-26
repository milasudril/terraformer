#ifndef TERRAFORMER_RESOURCE_TABLE_HPP
#define TERRAFORMER_RESOURCE_TABLE_HPP

#include <unordered_map>
#include <memory>

namespace terraformer
{
	template<class Key, class Value>
	class resource_table:private std::unordered_map<Key, std::shared_ptr<Value>>
	{
		using base = std::unordered_map<Key, std::shared_ptr<Value>>;

	public:
		using base::base;
		using base::contains;
		using base::insert;
		using base::emplace;
		using base::try_emplace;
		using base::insert_or_assign;

		template<class KeyType>
		std::shared_ptr<Value const> at_ptr(KeyType&& key) const
		{
			auto const i = base::find(key);
			return i != base::end()? i->second : std::shared_ptr<Value const>{};
		}

		template<class KeyType>
		std::shared_ptr<Value> at_ptr(KeyType&& key)
		{
			auto const i = base::find(key);
			return i != base::end()? i->second : std::shared_ptr<Value>{};
		}

		template<class KeyType1, class KeyType2>
		std::pair<std::shared_ptr<Value>, bool> create_or_replace_link(KeyType1&& old_key, KeyType2&& new_key)
		{
			auto const i_old = base::find(old_key);
			if(i_old == base::end())
			{ return std::pair<Value*, bool>{nullptr, false}; }

			auto const i = base::insert_or_assign(
				std::forward<KeyType2>(new_key),
				i->second
			);

			return std::pair{i_old->second, i.second};
		}

		template<class KeyType1, class KeyType2>
		std::pair<std::shared_ptr<Value>, bool> create_link(KeyType1&& old_key, KeyType2&& new_key)
		{
			auto const i_old = base::find(old_key);
			if(i_old == base::end())
			{ return std::pair<std::shared_ptr<Value>, bool>{nullptr, false}; }

			auto const i = base::insert(
				std::forward<KeyType2>(new_key),
				i->second
			);

			return std::pair{i_old->second, i.second};
		}
	};
}

#endif