#ifndef TERRAFORMER_STRING_TO_VALUE_MAP_HPP
#define TERRAFORMER_STRING_TO_VALUE_MAP_HPP

#include <unordered_map>
#include <string>
#include <string_view>

namespace terraformer
{
	template<class CharType>
	struct basic_string_hash
	{
		using is_transparent = void;
		[[nodiscard]] size_t operator()(char const* txt) const
		{ return std::hash<std::basic_string_view<CharType>>{}(txt); }

		[[nodiscard]] size_t operator()(std::basic_string_view<CharType> txt) const
		{ return std::hash<std::basic_string_view<CharType>>{}(txt); }

		[[nodiscard]] size_t operator()(std::string const& txt) const
		{ return std::hash<std::string>{}(txt); }
	};

	using string_hash = basic_string_hash<char>;

	template<class CharType, class MappedType>
	using basic_string_to_value_map = std::unordered_map<
		std::basic_string<CharType>,
		MappedType,
		basic_string_hash<CharType>,
		std::equal_to<>
	>;

	template<class MappedType>
	using u8string_to_value_map = basic_string_to_value_map<char8_t, MappedType>;
}
#endif