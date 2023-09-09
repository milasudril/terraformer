//@	{"target":{"name":"formgen_test"}}

#include <type_traits>
#include <string>
#include <charconv>
#include <stdexcept>

#include <limits>
#include <functional>

template<class WidgetType>
struct field
{
	char const* name;
	char const* display_name;
	char const* description;
	WidgetType widget;
};

template<class T>
struct open_open_interval{
	using value_type = T;
	T min;
	T max;
};

template<class T>
bool within(open_open_interval<T> range, T val)
{ return val > range.min && val < range.max; }

template<class T>
struct closed_open_interval{
	using value_type = T;
	T min;
	T max;
};

template<class T>
bool within(closed_open_interval<T> range, T val)
{ return val >= range.min && val < range.max; }

template<class T>
struct open_closed_interval{
	using value_type = T;
	T min;
	T max;
};

template<class T>
bool within(open_closed_interval<T> range, T val)
{ return val > range.min && val <= range.max; }

template<class T>
struct closed_closed_interval{
	using value_type = T;
	T min;
	T max;
};

template<class T>
bool within(closed_closed_interval<T> range, T val)
{ return val >= range.min && val <= range.max; }

class input_error:public std::runtime_error
{
public:
	explicit input_error(std::string&& str):std::runtime_error{std::move(str)}{}
};

template<class ValidRange>
requires std::is_arithmetic_v<typename ValidRange::value_type>
struct string_converter
{
	using deserialized_type = typename ValidRange::value_type;
	ValidRange range;

	static std::string to_string(deserialized_type value)
	{
		std::array<char, 32> buffer{};
		std::to_chars(std::begin(buffer), std::end(buffer), value);
		return std::string{std::data(buffer)};
	}

	deserialized_type from_string(std::string_view str)
	{
		deserialized_type val{};
		auto const res = std::from_chars(std::begin(str), std::end(str), val);
		switch(res.ec)
		{
			case std::errc{}:
				if(within(range, val))
				{ return val; }
				throw input_error{"Input value is out of range"};

			case std::errc::result_out_of_range:
				throw input_error{"Input value is out of range"};

			default:
				throw input_error{"Expected a number"};
		}

		return range.min;
	}
};


struct domain_size
{
	float width;
	float height;
	int scanline_count;
};

template<class Form>
void bind(Form& form, domain_size& dom_size)
{
	form.insert(
		field{
			.name = "width",
			.display_name = "Width",
			.description = "Sets the width of the domain",
			.widget = typename Form::textbox{
				.value_converter = string_converter{
					.range = open_open_interval{
						.min = 0.0f,
						.max = std::numeric_limits<float>::infinity(),
					}
				},
				.binding = std::ref(dom_size.width)
			}
		}
	);

	form.insert(
		field{
			.name = "height",
			.display_name = "Height",
			.description = "Sets the width of the domain",
			.widget = typename Form::textbox{
				.value_converter = string_converter{
					.range = open_open_interval{
						.min = 0.0f,
						.max = std::numeric_limits<float>::infinity(),
					}
				},
				.binding = std::ref(dom_size.height)
			}
		}
	);

	form.insert(
		field{
			.name = "scanline_count",
			.display_name = "Number of scanlines",
			.description = "Sets the number of scanlines in the generated images",
			.widget = typename Form::textbox{
				.value_converter = string_converter{
					.range = closed_closed_interval{
						.min = 1,
						.max = 8192,
					}
				},
				.binding = std::ref(dom_size.scanline_count)
			}
		}
	);
}

int main()
{}