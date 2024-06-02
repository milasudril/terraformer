#include "./shared_any.hpp"
#include "lib/array_classes/single_array.hpp"

#include <string>
#include <unordered_map>
#include <variant>

namespace terraformer
{
	struct string_hash
	{
		using is_transparent = void;
		[[nodiscard]] size_t operator()(char const* txt) const
		{ return std::hash<std::string_view>{}(txt); }

		[[nodiscard]] size_t operator()(std::string_view txt) const
		{ return std::hash<std::string_view>{}(txt); }

		[[nodiscard]] size_t operator()(std::string const& txt) const
		{ return std::hash<std::string>{}(txt); }
	};

	class object_tree;

	class object_array: private single_array<shared_any>
	{
	public:
		using base = single_array<shared_any>;
		using base::index_type;
		using base::size_type;
		using base::begin;
		using base::end;
		using base::size;
		using base::operator[];

		template<class TypeOfValueToInsert, class ... Args>
		object_array& append(Args&&... args)
		{
			base::push_back(
				shared_any{
					std::type_identity<TypeOfValueToInsert>{},
					std::forward<Args>(args)...
				}
			);
			return *this;
		}

		inline object_tree operator/(size_t index);
	};

	class object_dict: private std::unordered_map<std::string, shared_any, string_hash, std::equal_to<>>
	{
	public:
		using base = std::unordered_map<std::string, shared_any, string_hash, std::equal_to<>>;
		using base::begin;
		using base::end;
		using base::size;

		template<class TypeOfValueToInsert, class KeyType, class ... Args>
		object_dict& insert(KeyType&& key, Args&&... args)
		{
			auto res = base::emplace(
				std::forward<KeyType>(key),
				shared_any{std::type_identity<TypeOfValueToInsert>{}, std::forward<Args>(args)...}
			);

			if(res.second == false)
			{ throw std::runtime_error{"Key already exists"}; }

			return *this;
		}

		template<class TypeOfValueToInsert, class KeyType, class ... Args>
		object_dict& insert_or_assign(KeyType&& key, Args&&... args)
		{
			base::insert_or_assign(
				std::forward<KeyType>(key),
				shared_any{std::type_identity<TypeOfValueToInsert>{}, std::forward<Args>(args)...}
			);
			return *this;
		}

		template<class KeyType>
		inline object_tree operator/(KeyType&& key);
	};

	class object_tree
	{
	public:
		using map_type = object_dict;
		using array_type = object_array;

		object_tree(object_tree const&) = delete;
		object_tree& operator=(object_tree const&) = delete;
		object_tree(object_tree&&) = default;
		object_tree& operator=(object_tree&&) = default;

		explicit object_tree() = default;

		explicit object_tree(shared_any&& obj): m_value{std::move(obj)}{}

		explicit object_tree(shared_any const& obj):m_value{obj}{}

		template<class TypeOfValueToInsert, class ... Args>
		object_tree append(Args&&... args)
		{
			if(!m_value)
			{ m_value = shared_any{std::type_identity<array_type>{}}; }

			if(auto const array = m_value.template get_if<array_type>(); array != nullptr)
			{
				array->template append<TypeOfValueToInsert>(std::forward<Args>(args)...);
				return object_tree{m_value};
			}

			return object_tree{};
		}

		template<class TypeOfValueToInsert, class KeyType, class ... Args>
		object_tree insert(KeyType&& key, Args&&... args)
		{
			if(!m_value)
			{ m_value = shared_any{std::type_identity<map_type>{}}; }

			if(auto const map = m_value.template get_if<map_type>(); map != nullptr)
			{
				map->template insert<TypeOfValueToInsert>(
					std::forward<KeyType>(key),
					std::forward<Args>(args)...
				);
				return object_tree{m_value};
			}

			return object_tree{};
		}

		template<class TypeOfValueToInsert, class KeyType, class ... Args>
		object_tree insert_or_assign(KeyType&& key, Args&&... args)
		{
			if(!m_value)
			{ m_value = shared_any{std::type_identity<map_type>{}}; }

			if(auto const map = m_value.template get_if<map_type>(); map != nullptr)
			{
				map->template insert_or_assign<TypeOfValueToInsert>(
					std::forward<KeyType>(key),
					std::forward<Args>(args)...
				);
				return object_tree{m_value};
			}

			return object_tree{};
		}


		bool is_null() const
		{ return !m_value; }

		//FIXME: Add `const` support

		template<class T>
		operator T*()
		{ return m_value.template get_if<T>(); }

		object_tree operator/(std::string_view key)
		{
			auto const val = m_value.template get_if<map_type>();
			if(val == nullptr)
			{ return object_tree{}; }

			return (*val)/key;
		}

		object_tree operator/(size_t index)
		{
			auto const val = m_value.template get_if<array_type>();
			if(val == nullptr)
			{ return object_tree{}; }

			return (*val)/index;
		}

		template<class Func>
		void visit_elements(Func&& f)
		{
			if(auto const map = m_value.template get_if<map_type>(); map != nullptr)
			{
				for(auto& item : *map)
				{ f(item.first, object_tree{item.second}); }
			}
			else
			if(auto const array = m_value.template get_if<array_type>(); array != nullptr)
			{
				auto const n = std::size(*array);
				span const elems{array->begin(), array->end()};
				for(auto k = elems.first_element_index(); k != n; ++k)
				{ f(k, object_tree{elems[k]}); }
			}
			else
			{ f(object_tree{m_value}); }
		}

		size_t non_recursive_size() const
		{
			if(auto const map = m_value.template get_if<map_type>(); map != nullptr)
			{ return std::size(*map); }
			else
			if(auto const array = m_value.template get_if<array_type>(); array != nullptr)
			{ return std::size(*array).get(); }
			else
			{ return m_value? static_cast<size_t>(1) : 0; }
		}

	private:
		shared_any m_value;
	};

	object_tree object_array::operator/(size_t index)
	{
		return index < std::size(*this).get()? object_tree{(*this)[index_type{index}]} : object_tree{};
	}

	template<class KeyType>
	object_tree object_dict::operator/(KeyType&& key)
	{
		auto const i = base::find(std::forward<KeyType>(key));
		return i != std::end(*this)? object_tree{i->second} : object_tree{};
	}
}