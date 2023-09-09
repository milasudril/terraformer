//@	{
//@		"target":{
//@			"name":"formgen_test",
//@			"dependencies":[
//@				{"ref":"Qt5Widgets", "origin":"pkg-config"}
//@			]
//@		},
//@		"compiler":{
//@			"config": {
//@				"cflags":[
//	NOTE: We need to change some compiler flags for Qt
//@					"-Wno-error=conversion",
//@					"-Wno-error=deprecated-enum-enum-conversion",
//@					"-fpic"
//@				]
//@			}
//@		}
//@	}

#include <type_traits>
#include <string>
#include <charconv>
#include <stdexcept>

#include <limits>
#include <functional>

#include <QApplication>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

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

	deserialized_type from_string(std::string_view str) const
	{
		deserialized_type val{};
		auto const res = std::from_chars(std::begin(str), std::end(str), val);
		if(res.ec == std::errc{})
		{
			if(within(range, val))
			{ return val; }
			throw input_error{"Input value is out of range"};
		}

		switch(res.ec)
		{
			case std::errc::result_out_of_range:
				throw input_error{"Input value is out of range"};

			default:
				throw input_error{"Expected a number"};
		}
	}
};

template<class Widget>
struct field
{
	char const* name;
	char const* display_name;
	char const* description;
	Widget widget;
};

template<class Converter, class BindingType>
struct textbox
{
	Converter value_converter;
	BindingType binding;
};

template<class Callable, class BindingType>
struct text_display
{
	Callable text_source;
	BindingType binding;
};

struct domain_size
{
	float width;
	float height;
	int scanline_count;
};

float compute_pixel_size(domain_size const& dom_size)
{
	return dom_size.height/static_cast<float>(dom_size.scanline_count);
}

template<class Form>
void bind(Form& form, domain_size& dom_size)
{
	form.insert(
		field{
			.name = "width",
			.display_name = "Width",
			.description = "Sets the width of the domain",
			.widget = textbox{
				.value_converter = string_converter{
					.range = open_open_interval{
						.min = 0.0f,
						.max = std::numeric_limits<float>::infinity()
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
			.widget = textbox{
				.value_converter = string_converter{
					.range = open_open_interval{
						.min = 0.0f,
						.max = std::numeric_limits<float>::infinity()
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
			.widget = textbox{
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

	form.insert(
		field{
			.name = "pixel_size",
			.display_name = "Pixel size",
			.description = "The physical size of a pixel",
			.widget = text_display{
				.text_source = [](domain_size const& dom_size){
					return std::to_string(compute_pixel_size(dom_size));
				},
				.binding = std::cref(dom_size)
			}
		}
	);
}

class qt_form
{
public:
	template<class ErrorHandler>
	qt_form(QWidget* parent, ErrorHandler&& error_handler):
		m_root{parent},
		m_error_handler{std::forward<ErrorHandler>(error_handler)}
	{}

	template<class FieldDescriptor>
	void insert(FieldDescriptor&& field)
	{
		m_widgets.push_back(create_widget(std::move(field.widget)));
		m_root.addRow(field.display_name, m_widgets.back().get());
	}

	template<class Converter, class BindingType>
	std::unique_ptr<QWidget> create_widget(textbox<Converter, BindingType>&& textbox)
	{
		auto ret = std::make_unique<QLineEdit>();
		QObject::connect(ret.get(),
			&QLineEdit::editingFinished,
			[&src = *ret, textbox = std::move(textbox), this](){
				try
				{
					auto const str = src.text().toStdString();
					textbox.binding.get() = textbox.value_converter.from_string(str);
					std::ranges::for_each(m_display_callbacks, [](auto const& item){item();});
				}
				catch(std::runtime_error const& err)
				{ m_error_handler(err.what()); }
			}
		);
		return ret;
	}

	template<class Converter, class BindingType>
	std::unique_ptr<QWidget> create_widget(text_display<Converter, BindingType>&& text_display)
	{
		auto ret = std::make_unique<QLabel>();
		m_display_callbacks.push_back([&dest = *ret, text_display = std::move(text_display)](){
			dest.setText(text_display.text_source(text_display.binding.get()).c_str());
		});
		return ret;
	}


private:
	std::vector<std::unique_ptr<QWidget>> m_widgets;
	std::vector<std::function<void()>> m_display_callbacks;
	QFormLayout m_root;
	std::function<void(char const*)> m_error_handler;
};

int main(int argc, char** argv)
{
	QApplication my_app{argc, argv};
	QWidget mainwin;
	qt_form my_form{&mainwin, [](char const* err){
		fprintf(stderr, "Error: %s\n", err);
	}};

	domain_size dom;
	bind(my_form, dom);
	mainwin.show();

	my_app.exec();
}