#ifndef TERRAFORMER_RESOURCE_TABLE_HPP
#define TERRAFORMER_RESOURCE_TABLE_HPP

#include "./shared_any.hpp"
#include "lib/array_classes/single_array.hpp"

#include <unordered_map>
#include <string>
#include <cassert>

namespace terraformer
{
	class resource_list
	{
	public:
		using index_type = single_array<shared_any>::index_type;

		[[nodiscard]] auto operator[](index_type index) const
		{ return m_values[index].get_const(); }

		[[nodiscard]] auto operator[](index_type index)
		{ return m_values[index].get(); }

		template<class T>
		[[nodiscard]] T const* get_if(index_type index) const
		{ return (*this)[index]; }

		template<class T>
		[[nodiscard]] T* get_if(index_type index)
		{ return (*this)[index]; }

		template<class Value, class ... Args>
		void emplace_back(Args&&... args)
		{
			return m_values.push_back(
				shared_any{
					std::type_identity<Value>{},
					std::forward<Args>(args)...
				}
			);
		}

		auto size() const
		{ return std::size(m_values); }

	private:
		single_array<shared_any> m_values;
	};

	class resource_table:private std::unordered_map<std::string, shared_any>
	{
		using base = std::unordered_map<std::string, shared_any>;

	public:
		using base::base;
		using base::contains;

		template<class KeyType>
		[[nodiscard]] auto get_if(KeyType&& key) const
		{
			auto const i = base::find(key);
			return i != base::end()? i->second.get_const() : shared_any::value_const{};
		}

		template<class KeyType>
		[[nodiscard]] auto get_if(KeyType&& key)
		{
			auto const i = base::find(key);
			return i != base::end()? i->second.get() : shared_any::value{};
		}

		template<class Value, class KeyType>
		[[nodiscard]] Value const* get_if(KeyType&& key) const
		{ return get_if(std::forward<KeyType>(key)); }

		template<class Value, class KeyType>
		[[nodiscard]] Value* get_if(KeyType&& key)
		{ return get_if(std::forward<KeyType>(key)); }

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