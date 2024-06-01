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
		[[nodiscard]] size_t operator()(const char *txt) const
		{ return std::hash<std::string_view>{}(txt); }

		[[nodiscard]] size_t operator()(std::string_view txt) const
		{ return std::hash<std::string_view>{}(txt); }

		[[nodiscard]] size_t operator()(const std::string &txt) const
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

	class object_tree
	{
	public:
		using map_type = std::unordered_map<std::string, shared_any, string_hash, std::equal_to<>>;
		using array_type = object_array;

		template<class T>
		static constexpr auto is_leaf_type_v = !std::is_same_v<std::remove_cvref_t<T>, object_tree>;

		object_tree(object_tree const&) = delete;
		object_tree& operator=(object_tree const&) = delete;
		object_tree(object_tree&&) = default;
		object_tree& operator=(object_tree&&) = default;

		explicit object_tree() = default;

		explicit object_tree(shared_any&& obj): m_value{std::move(obj)}{}

		explicit object_tree(shared_any const& obj):m_value{obj}{}

		template<class TypeOfValueToInsert, class ... Args>
		requires(is_leaf_type_v<TypeOfValueToInsert>)
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

		bool is_null() const
		{ return !m_value; }

		//FIXME: Add `const` support

		template<class T>
		requires(is_leaf_type_v<T>)
		operator T*()
		{ return m_value.template get_if<T>(); }

		object_tree operator/(std::string_view key)
		{
			auto const val = m_value.template get_if<map_type>();
			if(val == nullptr)
			{ return object_tree{}; }

			auto const i = val->find(key);
			return i != std::end(*val)? object_tree{i->second} : object_tree{};
		}

		object_tree operator/(size_t index)
		{
			using index_type = array_type::index_type;

			auto const val = m_value.template get_if<array_type>();
			if(val == nullptr)
			{ return object_tree{}; }

			return index < std::size(*val).get()? object_tree{(*val)[index_type{index}]} : object_tree{};
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
}