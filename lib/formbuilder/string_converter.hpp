#ifndef TERRAFORMER_STRING_CONVERTER_HPP
#define TERRAFORMER_STRING_CONVERTER_HPP

#include "lib/common/interval.hpp"

namespace terraformer
{
	class input_error:public std::runtime_error
	{
	public:
		explicit input_error(std::string str):std::runtime_error{std::move(str)}{}
	};

	template<class ValidRange, class T = typename ValidRange::value_type>
	requires interval<ValidRange, T>
	struct num_string_converter
	{
		using deserialized_type = typename ValidRange::value_type;
		ValidRange range;

		static std::string convert(deserialized_type value)
		{ return to_string_helper(value); }

		deserialized_type convert(std::string_view str) const
		{
			deserialized_type val{};
			auto const res = std::from_chars(std::begin(str), std::end(str), val);

			if(res.ptr != std::end(str))
			{ throw input_error{"Expected a number."}; }

			if(res.ec == std::errc{})
			{
				if(within(range, val))
				{ return val; }
				throw input_error{std::string{"Input value is out of range. Valid range is "}.append(to_string(range)).append(".")};
			}

			switch(res.ec)
			{
				case std::errc::result_out_of_range:
				throw input_error{std::string{"Input value is out of range. Valid range is "}.append(to_string(range)).append(".")};

				default:
					throw input_error{"Expected a number"};
			}
		}
	};

	template<class T>
	requires std::is_integral_v<T> || std::is_same_v<T, __int128>
	struct hash_string_converter
	{
		static std::string convert(T value)
		{
			std::array<char, 2*sizeof(T)> buffer;
			bytes_to_hex(std::data(buffer), &value, 2*sizeof(T));
			return std::string{std::data(buffer), std::size(buffer)};
		}

		static T convert(std::string_view buffer)
		{
			if(std::size(buffer) != 2*sizeof(T))
			{
				throw input_error{
					std::string{"Input buffer must be exactly "}
						.append(std::to_string(2*sizeof(T)))
						.append(" bytes long.")
				};
			}

			T ret;
			auto const res = hex_to_bytes(&ret, std::data(buffer), sizeof(T));
			if(res.ptr != std::data(buffer) + std::size(buffer))
			{
				throw input_error{"Input buffer must be a hexadecimal number, without any prefix or suffix."};
			}

			return ret;
		}
	};
}

#endif