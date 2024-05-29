#ifndef TERRAFORMER_RESOURCE_TABLE_HPP
#define TERRAFORMER_RESOURCE_TABLE_HPP

#include "./shared_any.hpp"

#include <unordered_map>
#include <string>
#include <cassert>

namespace terraformer
{
	class resource_table:private std::unordered_map<std::string, shared_any>
	{
		using base = std::unordered_map<std::string, shared_any>;

	public:
		using base::base;
		using base::contains;

		template<class Value, class KeyType>
		[[nodiscard]] Value const* get_if(KeyType&& key) const
		{
			auto const i = base::find(key);
			return i != base::end()? i->second.template get_if<Value>() : nullptr;
		}

		template<class Value, class KeyType>
		[[nodiscard]] Value* get_if(KeyType&& key)
		{
			auto const i = base::find(key);
			return i != base::end()? i->second.template get_if<Value>() : nullptr;
		}

		template<class Value, class KeyType, class ... Args>
		[[nodiscard("Verify that insertion took place")]]
		std::pair<Value*, bool> emplace(KeyType&& key, Args&&... args)
		{
			auto const i = base::emplace(
				std::forward<KeyType>(key),
				shared_any{std::type_identity<Value>{},std::forward<Args>(args)...}
			);
			return std::pair{i.first->second.template get_if<Value>(), i.second};
		}

		template<class Value, class KeyType, class... Args>
		[[nodiscard("Verify that insertion took place")]]
		std::pair<Value*, bool> insert_or_assign_linked(KeyType&& key, Args&&... args)
		{
			auto const i = base::find(key);
			if(i == base::end())
			{ return emplace<Value>(std::forward<KeyType>(key), std::forward<Args>(args)...); }

			auto ptr = i->second.template get_if<Value>();
			if(ptr == nullptr)
			{ return std::pair{nullptr, false}; }

			*ptr = Value{std::forward<Args>(args)...};
			return std::pair{ptr, false};
		}

		template<class KeyType1, class KeyType2>
		[[nodiscard("Verify that a link was created")]]
		bool create_link(KeyType1&& old_key, KeyType2&& new_key)
		{
			assert(old_key != new_key);

			auto const i_old = base::find(old_key);
			if(i_old == base::end())
			{ return false; }

			auto const i = base::insert(
				typename base::value_type{
					std::forward<KeyType2>(new_key),
					i_old->second
				}
			);

			return i.second;
		}
	};
}

#endif