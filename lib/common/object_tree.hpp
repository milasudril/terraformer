#ifndef TERRAFORMER_OBJECT_TREE_HPP
#define TERRAFORMER_OBJECT_TREE_HPP

#include "lib/any/shared_any.hpp"
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

		object_array() = default,
		object_array(object_array&&) = default;
		object_array& operator=(object_array&&) = default;
		object_array(object_array const&) = delete;
		object_array& operator=(object_array const&) = delete;

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

		template<class ValueType>
		object_array& append_link(ValueType&& val)
		{
			base::push_back(std::forward<ValueType>(val));
			return *this;
		}

		inline object_pointer<false> operator/(size_t index);

		inline object_pointer<true> operator/(size_t index) const;

		inline shared_const_any dup(size_t index) const;

		template<class Function>
		void visit_elements(Function&& f);

		template<class Function>
		void visit_elements(Function&& f) const;
	};

	class object_dict: private std::unordered_map<std::string, shared_any, string_hash, std::equal_to<>>
	{
	public:
		using base = std::unordered_map<std::string, shared_any, string_hash, std::equal_to<>>;
		using base::begin;
		using base::end;
		using base::size;
		using base::empty;

		object_dict() = default,
		object_dict(object_dict&&) = default;
		object_dict& operator=(object_dict&&) = default;
		object_dict(object_dict const&) = delete;
		object_dict& operator=(object_dict const&) = delete;

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

		template<class KeyType, class ValueType>
		object_dict& insert_link(KeyType&& key, ValueType&& val)
		{
			auto res = base::emplace(
				std::forward<KeyType>(key),
				std::forward<ValueType>(val)
			);

			if(res.second == false)
			{ throw std::runtime_error{"Key already exists"}; }

			return *this;
		}

		template<class KeyType>
		inline object_pointer<false> operator/(KeyType&& key);

		template<class KeyType>
		inline object_pointer<true> operator/(KeyType&& key) const;

		template<class KeyType>
		inline shared_const_any dup(KeyType&& key) const;

		template<class Function>
		void visit_elements(Function&& f);

		template<class Function>
		void visit_elements(Function&& f) const;

	};

	template<bool IsConst>
	class object_pointer
	{
	public:
		using dict_type = std::conditional_t<IsConst, object_dict const, object_dict>;
		using array_type = std::conditional_t<IsConst, object_array const, object_array>;

		explicit object_pointer() = default;

		template<class Dummy = void>
		requires(IsConst)
		object_pointer(object_pointer<false> other):m_pointer{other.pointer()}{}

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

		template<class ValueType>
		requires(!IsConst)
		object_pointer append_link(ValueType&& val)
		{
			if(auto const array = m_pointer.template get_if<array_type>(); array != nullptr)
			{
				array->append_link(std::forward<ValueType>(val));
				return *this;
			}

			return object_pointer{};
		}

		template<class TypeOfValueToInsert, class KeyType, class ... Args>
		requires(!IsConst)
		object_pointer insert(KeyType&& key, Args&&... args) const
		{
			if(auto const dict = m_pointer.template get_if<dict_type>(); dict != nullptr)
			{
				dict->template insert<TypeOfValueToInsert>(
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
			if(auto const dict = m_pointer.template get_if<dict_type>(); dict != nullptr)
			{
				dict->template insert_or_assign<TypeOfValueToInsert>(
					std::forward<KeyType>(key),
					std::forward<Args>(args)...
				);
				return *this;
			}

			return object_pointer{};
		}

		template<class KeyType, class ValueType>
		object_pointer insert_link(KeyType&& key, ValueType&& val)
		{
			if(auto const dict = m_pointer.template get_if<dict_type>(); dict != nullptr)
			{
				dict->insert_link(
					std::forward<KeyType>(key),
					std::forward<ValueType>(val)
				);
				return *this;
			}

			return object_pointer{};
		}

		bool is_null() const
		{ return !m_pointer; }

		template<class T>
		operator T*() const
		{ return static_cast<T*>(m_pointer); }

		template<class T>
		T* get_if() const
		{ return static_cast<T*>(m_pointer); }

		object_pointer operator/(std::string_view key) const
		{
			auto const val = m_pointer.template get_if<dict_type>();
			if(val == nullptr)
			{ return object_pointer{}; }

			return (*val)/key;
		}

		auto dup(std::string_view const key) const
		{
			auto const val = m_pointer.template get_if<dict_type>();
			if(val == nullptr)
			{ return shared_const_any{}; }

			return val->dup(key);
		}

		object_pointer operator/(size_t index) const
		{
			auto const val = m_pointer.template get_if<array_type>();
			if(val == nullptr)
			{ return object_pointer{}; }

			return (*val)/index;
		}

		auto dup(size_t index) const
		{
			auto const val = m_pointer.template get_if<array_type>();
			if(val == nullptr)
			{ return shared_const_any{}; }

			return val->dup(index);
		}

		template<class Func>
		void visit_elements(Func&& f) const
		{
			if(auto const dict = m_pointer.template get_if<dict_type>(); dict != nullptr)
			{ dict->template visit_elements(std::forward<Func>(f)); }
			else
			if(auto const array = m_pointer.template get_if<array_type>(); array != nullptr)
			{ array->template visit_elements(std::forward<Func>(f)); }
			else
			{ std::forward<Func>(f)(object_pointer<IsConst>{m_pointer}); }
		}

		size_t size() const
		{
			if(auto const dict = m_pointer.template get_if<dict_type>(); dict != nullptr)
			{ return std::size(*dict); }
			else
			if(auto const array = m_pointer.template get_if<array_type>(); array != nullptr)
			{ return std::size(*array).get(); }
			else
			{ return m_pointer? static_cast<size_t>(1) : 0; }
		}

		auto operator<=>(object_pointer const&) const = default;

		auto pointer() const
		{ return m_pointer; }

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

	shared_const_any object_array::dup(size_t index) const
	{
		return index < std::size(*this).get()?
			shared_const_any{(*this)[index_type{index}]} :
			shared_const_any{};
	}

	template<class Function>
	void object_array::visit_elements(Function&& f)
	{
		auto const n = std::size(*this);
		span const elems{begin(), end()};
		for(auto k = elems.first_element_index(); k != n; ++k)
		{ f(k, object_pointer{elems[k].get()}); }
	}

	template<class Function>
	void object_array::visit_elements(Function&& f) const
	{
		auto const n = std::size(*this);
		span const elems{begin(), end()};
		for(auto k = elems.first_element_index(); k != n; ++k)
		{ f(k, object_pointer{elems[k].get_const()}); }
	}

	template<class KeyType>
	object_pointer<false> object_dict::operator/(KeyType&& key)
	{
		auto const i = base::find(std::forward<KeyType>(key));
		return i != std::end(*this)? object_pointer{i->second.get()} : object_pointer<false>{};
	}

	template<class KeyType>
	object_pointer<true> object_dict::operator/(KeyType&& key) const
	{
		auto const i = base::find(std::forward<KeyType>(key));
		return i != std::end(*this)? object_pointer{i->second.get_const()} : object_pointer<true>{};
	}

	template<class KeyType>
	shared_const_any object_dict::dup(KeyType&& key) const
	{
		auto const i = base::find(std::forward<KeyType>(key));
		return i != std::end(*this)? shared_const_any{i->second} : shared_const_any{};
	}

	template<class Function>
	void object_dict::visit_elements(Function&& f)
	{
		for(auto& item : *this)
		{ f(item.first, object_pointer{item.second.get()}); }
	}

	template<class Function>
	void object_dict::visit_elements(Function&& f) const
	{
		for(auto& item : *this)
		{ f(item.first, object_pointer{item.second.get_const()}); }
	}
}
#endif