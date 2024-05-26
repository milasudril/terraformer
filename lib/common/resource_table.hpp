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
		using base::find;
		using base::contains;

		template<class KeyType>
		Value const* at_ptr(KeyType&& key) const
		{
			auto const i = base::find(key);
			return i != base::end()? i->second.get() : nullptr;
		}

		template<class KeyType, class ... Args>
		std::pair<Value*, bool> emplace(KeyType&& key, Args&&... args)
		{
			auto const i = base::emplace(
				std::forward<KeyType>(key),
				std::make_shared<Value>(std::forward<Args>(args...))
			);

			return std::pair{i.first->second.get(), i.second};
		}

		template<class KeyType, class ... Args>
		std::pair<Value*, bool> emplace_or_assign(KeyType&& key, Args&&... args)
		{
			auto const i = base::insert_or_assign(
				std::forward<KeyType>(key),
				std::make_shared<Value>(std::forward<Args>(args...))
			);

			return std::pair{i.first->second.get(), i.second};
		}

		template<class KeyType1, class KeyType2>
		std::pair<Value*, bool> create_or_replace_link(KeyType1&& old_key, KeyType2&& new_key)
		{
			auto const i_old = base::find(old_key);
			if(i_old == base::end())
			{ return std::pair<Value*, bool>{nullptr, false}; }

			auto const i = base::insert_or_assign(
				std::forward<KeyType2>(new_key),
				i->second
			);

			return std::pair{i_old->second.get(), i.second};
		}

		template<class KeyType1, class KeyType2>
		std::pair<Value*, bool> create_link(KeyType1&& old_key, KeyType2&& new_key)
		{
			auto const i_old = base::find(old_key);
			if(i_old == base::end())
			{ return std::pair<Value*, bool>{nullptr, false}; }

			auto const i = base::insert(
				std::forward<KeyType2>(new_key),
				i->second
			);

			return std::pair{i_old->second.get(), i.second};
		}

		template<class KeyType, class ... Args>
		std::pair<Value*, bool> emplace_or_assign_linked(KeyType&& key, Args&&... args)
		{
			auto const i = base::find(key);
			if(i == base::end())
			{
				auto const res = base::insert(
					std::forward<KeyTpye>(key),
					std::make_shared<Value>(std::forward<Args>(args)...)
				);

				return std::pair{res.first->second, true};
			}

			*i->second = Value{std::forward<Args>(args)...};
			return std::pair{i->second.get(), false};
		}
	};
}

#endif