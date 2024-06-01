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

	class object_tree
	{
	public:
		explicit object_tree() = default;

		explicit object_tree(shared_any&& obj): m_value{std::move(obj)}{}

		explicit object_tree(shared_any const& obj):m_value{obj}{}

		using map_type = std::unordered_map<std::string, shared_any, string_hash, std::equal_to<>>;
		using array_type = single_array<shared_any>;
		using index_type = array_type::index_type;

		template<class T>
		operator T*()
		{ return m_value.template get_if<T>(); }

		auto operator/(std::string_view key)
		{
			auto const val = m_value.template get_if<map_type>();
			if(val == nullptr)
			{ return object_tree{}; }

			auto const i = val->find(key);
			return i != std::end(*val)? object_tree{i->second} : object_tree{};
		}

		auto operator/(index_type index)
		{
			auto const val = m_value.template get_if<array_type>();
			if(val == nullptr)
			{ return object_tree{}; }

			return index < std::size(*val)? object_tree{(*val)[index]} : object_tree{};
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

	private:
		shared_any m_value;
	};
}