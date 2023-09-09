//@	{"target":{"name":"formgen_test"}}

#include <type_traits>
#include <string>
#include <charconv>

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
requires std::is_arithmetic_v<T>
struct string_converter
{
	T min;
	T max;

	static std::string to_string(T value)
	{
		std::array<char, 32> buffer{};
		std::to_chars(std::begin(buffer), std::end(buffer), value);
		return std::string{std::data(buffer)};
	}

	T from_string(std::string_view str)
	{

		return min;
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
					.min = 0.0f,
					.max = std::numeric_limits<float>::infinity()
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
					.min = 0.0f,
					.max = std::numeric_limits<float>::infinity()
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
					.min = 1,
					.max = 8192
				},
				.binding = std::ref(dom_size.scanline_count)
			}
		}
	);
}

int main()
{}