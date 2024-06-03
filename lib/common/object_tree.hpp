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

	template<bool>
	class object_pointer;

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
		using base::empty;

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

		inline object_pointer<false> operator/(size_t index);

		inline object_pointer<true> operator/(size_t index) const;
	};

	class object_dict: private std::unordered_map<std::string, shared_any, string_hash, std::equal_to<>>
	{
	public:
		using base = std::unordered_map<std::string, shared_any, string_hash, std::equal_to<>>;
		using base::begin;
		using base::end;
		using base::size;
		using base::empty;

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
		inline object_pointer<false> operator/(KeyType&& key);
	};

	template<bool IsConst>
	class object_pointer
	{
	public:
		using map_type = object_dict;
		using array_type = object_array;

		explicit object_pointer() = default;

		explicit object_pointer(any_pointer<IsConst> pointer): m_pointer{pointer}{}

		template<class TypeOfValueToInsert, class ... Args>
		requires(!IsConst)
		object_pointer append(Args&&... args) const
		{
			if(auto const array = m_pointer.template get_if<array_type>(); array != nullptr)
			{
				array->template append<TypeOfValueToInsert>(std::forward<Args>(args)...);
				return *this;
			}

			return object_pointer{};
		}

		template<class TypeOfValueToInsert, class KeyType, class ... Args>
		requires(!IsConst)
		object_pointer insert(KeyType&& key, Args&&... args) const
		{
			if(auto const map = m_pointer.template get_if<map_type>(); map != nullptr)
			{
				map->template insert<TypeOfValueToInsert>(
					std::forward<KeyType>(key),
					std::forward<Args>(args)...
				);
				return *this;
			}

			return object_pointer{};
		}

		template<class TypeOfValueToInsert, class KeyType, class ... Args>
		requires(!IsConst)
		object_pointer insert_or_assign(KeyType&& key, Args&&... args) const
		{
			if(auto const map = m_pointer.template get_if<map_type>(); map != nullptr)
			{
				map->template insert_or_assign<TypeOfValueToInsert>(
					std::forward<KeyType>(key),
					std::forward<Args>(args)...
				);
				return *this;
			}

			return object_pointer{};
		}

		bool is_null() const
		{ return !m_pointer; }

		//FIXME: Add `const` support

		template<class T>
		operator T*() const
		{ return static_cast<T*>(m_pointer); }

		object_pointer operator/(std::string_view key) const
		{
			auto const val = m_pointer.template get_if<map_type>();
			if(val == nullptr)
			{ return object_pointer{}; }

			return (*val)/key;
		}

		object_pointer operator/(size_t index) const
		{
			auto const val = m_pointer.template get_if<array_type>();
			if(val == nullptr)
			{ return object_pointer{}; }

			return (*val)/index;
		}

		template<class Func>
		void visit_elements(Func&& f) const
		{
			if(auto const map = m_pointer.template get_if<map_type>(); map != nullptr)
			{
				for(auto& item : *map)
				{ f(item.first, object_pointer{item.second}); }
			}
			else
			if(auto const array = m_pointer.template get_if<array_type>(); array != nullptr)
			{
				auto const n = std::size(*array);
				span const elems{array->begin(), array->end()};
				for(auto k = elems.first_element_index(); k != n; ++k)
				{ f(k, object_pointer{elems[k]}); }
			}
			else
			{ f(object_pointer{m_pointer}); }
		}

		size_t non_recursive_size() const
		{
			if(auto const map = m_pointer.template get_if<map_type>(); map != nullptr)
			{ return std::size(*map); }
			else
			if(auto const array = m_pointer.template get_if<array_type>(); array != nullptr)
			{ return std::size(*array).get(); }
			else
			{ return m_pointer? static_cast<size_t>(1) : 0; }
		}

	private:
		any_pointer<IsConst> m_pointer;
	};

	object_pointer(any_pointer<true>) -> object_pointer<true>;
	object_pointer(any_pointer<false>) -> object_pointer<false>;

	object_pointer<false> object_array::operator/(size_t index)
	{
		return index < std::size(*this).get()?
			object_pointer{(*this)[index_type{index}].get()} :
			object_pointer<false>{};
	}

	object_pointer<true> object_array::operator/(size_t index) const
	{
		return index < std::size(*this).get()?
			object_pointer{(*this)[index_type{index}].get_const()} :
			object_pointer<true>{};
	}

	template<class KeyType>
	object_pointer<false> object_dict::operator/(KeyType&& key)
	{
		auto const i = base::find(std::forward<KeyType>(key));
		return i != std::end(*this)? object_pointer{i->second.get()} : object_pointer<false>{};
	}
}