//@	{
//@		"target":{
//@			"name":"formgen_test",
//@			"dependencies":[
//@				{"ref":"Qt5Widgets", "origin":"pkg-config"}
//@			]
//@		},
//@		"dependencies_extra":[{"ref":"fdcb", "origin":"system", "rel":"external"}],
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
#include <cmath>

#include <QApplication>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>
#include <QTextEdit>
#include <QBoxLayout>
#include <fdcb.h>

template<class T>
requires std::is_arithmetic_v<T>
inline std::string to_string_helper(T value)
{
	std::array<char, 32> buffer{};
	std::to_chars(std::begin(buffer), std::end(buffer), value);
	return std::string{std::data(buffer)};
}

template<class IntervalType>
inline std::string to_string(IntervalType range)
{
	std::string ret{};
	ret += IntervalType::lower_bound_char;
	ret.append(to_string_helper(range.min)).append(", ").append(to_string_helper(range.max));
	ret += IntervalType::upper_bound_char;
	return ret;
}

template<class T>
struct open_open_interval{
	using value_type = T;
	T min;
	T max;
	static constexpr auto lower_bound_char = ']';
	static constexpr auto upper_bound_char = '[';
};

template<class T>
bool within(open_open_interval<T> range, T val)
{ return val > range.min && val < range.max; }

template<class T>
struct closed_open_interval{
	using value_type = T;
	T min;
	T max;
	static constexpr auto lower_bound_char = '[';
	static constexpr auto upper_bound_char = '[';
};

template<class T>
bool within(closed_open_interval<T> range, T val)
{ return val >= range.min && val < range.max; }

template<class T>
struct open_closed_interval{
	using value_type = T;
	T min;
	T max;
	static constexpr auto lower_bound_char = ']';
	static constexpr auto upper_bound_char = ']';
};

template<class T>
bool within(open_closed_interval<T> range, T val)
{ return val > range.min && val <= range.max; }

template<class T>
struct closed_closed_interval{
	using value_type = T;
	T min;
	T max;
	static constexpr auto lower_bound_char = '[';
	static constexpr auto upper_bound_char = ']';
};

template<class T>
bool within(closed_closed_interval<T> range, T val)
{ return val >= range.min && val <= range.max; }

class input_error:public std::runtime_error
{
public:
	explicit input_error(std::string str):std::runtime_error{std::move(str)}{}
};

template<class ValidRange>
requires std::is_arithmetic_v<typename ValidRange::value_type>
struct string_converter
{
	using deserialized_type = typename ValidRange::value_type;
	ValidRange range;

	static std::string to_string(deserialized_type value)
	{ return to_string_helper(value); }

	deserialized_type from_string(std::string_view str) const
	{
		deserialized_type val{};
		auto const res = std::from_chars(std::begin(str), std::end(str), val);

		if(res.ptr != std::end(str))
		{ throw input_error{"Expected a number"}; }

		if(res.ec == std::errc{})
		{
			if(within(range, val))
			{ return val; }
			throw input_error{std::string{"Input value is out of range. Valid range is "}.append(::to_string(range)).append(".")};
		}

		switch(res.ec)
		{
			case std::errc::result_out_of_range:
			throw input_error{std::string{"Input value is out of range. Valid range is "}.append(::to_string(range)).append(".")};

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
	Callable source;
	BindingType binding;
};

struct domain_size
{
	float width;
	float height;
	int number_of_pixels;
};

float compute_pixel_size(domain_size const& dom_size)
{
	return std::sqrt(dom_size.height*dom_size.width/static_cast<float>(dom_size.number_of_pixels));
}

int compute_image_width(domain_size const& dom_size)
{
	auto const ratio = static_cast<double>(dom_size.width)/static_cast<double>(dom_size.height);
	auto const pixel_area = static_cast<double>(dom_size.number_of_pixels);
	return static_cast<int>(std::sqrt(ratio*pixel_area));
}

int compute_image_height(domain_size const& dom_size)
{
	auto const ratio = static_cast<double>(dom_size.width)/static_cast<double>(dom_size.height);
	auto const pixel_area = static_cast<double>(dom_size.number_of_pixels);
	return static_cast<int>(std::sqrt(pixel_area/ratio));
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
			.name = "number_of_pixels",
			.display_name = "Number of pixel",
			.description = "Sets the number of pixels in the generated images",
			.widget = textbox{
				.value_converter = string_converter{
					.range = closed_closed_interval{
						.min = 1,
						.max = 8192*8192,
					}
				},
				.binding = std::ref(dom_size.number_of_pixels)
			}
		}
	);

	form.insert(
		field{
			.name = "image_width",
			.display_name = "Image width",
			.description = "The number of columns in the generated images",
			.widget = text_display{
				.source = [](domain_size const& dom_size){
					return to_string_helper(compute_image_width(dom_size));
				},
				.binding = std::cref(dom_size)
			}
		}
	);

	form.insert(
		field{
			.name = "image_height",
			.display_name = "Image height",
			.description = "The number of canlines in the generated images",
			.widget = text_display{
				.source = [](domain_size const& dom_size){
					return to_string_helper(compute_image_height(dom_size));
				},
				.binding = std::cref(dom_size)
			}
		}
	);

	form.insert(
		field{
			.name = "pixel_size",
			.display_name = "Pixel size",
			.description = "The physical size of a pixel",
			.widget = text_display{
				.source = [](domain_size const& dom_size){
					return to_string_helper(compute_pixel_size(dom_size));
				},
				.binding = std::cref(dom_size)
			}
		}
	);
}

template<class Context, class Callable, class ... Args>
decltype(auto) try_and_catch(Context context, Callable&& func, Args&&... args)
{
	try
	{ return std::invoke(std::forward<Callable>(func), std::forward<Args>(args)...); }
	catch(std::runtime_error const& error)
	{ context(error); }
}

void log_error(char const* msg)
{
	fprintf(stderr, "(x) %s\n", msg);
	fflush(stderr);
}

class qt_form
{
public:
	qt_form(QWidget* parent):
		m_root{parent}
	{}

	void set_focus()
	{ m_widgets[0]->setFocus(); }

	template<class FieldDescriptor>
	void insert(FieldDescriptor&& field)
	{
		m_widgets.push_back(create_widget(std::move(field.widget)));
		m_widgets.back()->setToolTip(field.description);
		m_root.addRow(field.display_name, m_widgets.back().get());
	}

	template<class Converter, class BindingType>
	std::unique_ptr<QWidget> create_widget(textbox<Converter, BindingType> const& textbox)
	{
		auto ret = std::make_unique<QLineEdit>();
		QObject::connect(ret.get(),
			&QLineEdit::editingFinished,
			[this, &src = *ret, textbox](){
				try_and_catch([&src](auto const& error){
					log_error(error.what());
					src.setFocus();
				}, [this](auto& src, auto const& textbox){
					auto const str = src.text().toStdString();
					textbox.binding.get() = textbox.value_converter.from_string(str);
				}, src, textbox);
				refresh();
			}
		);
		m_display_callbacks.push_back([&dest = *ret, textbox](){
			dest.setText(textbox.value_converter.to_string(textbox.binding.get()).c_str());
		});

		return ret;
	}

	template<class Converter, class BindingType>
	std::unique_ptr<QWidget> create_widget(text_display<Converter, BindingType>&& text_display)
	{
		auto ret = std::make_unique<QLabel>();
		m_display_callbacks.push_back([&dest = *ret, text_display = std::move(text_display)](){
			try_and_catch([](auto const& error){
				log_error(error.what());
			},[](auto& dest, auto const& text_display) {
				dest.setText(text_display.source(text_display.binding.get()).c_str());
			}, dest, text_display);
		});
		return ret;
	}

	void refresh()
	{ std::ranges::for_each(m_display_callbacks, [](auto const& item){item();}); }


private:
	std::vector<std::unique_ptr<QWidget>> m_widgets;
	std::vector<std::function<void()>> m_display_callbacks;
	QFormLayout m_root;
	std::function<void(char const*)> m_error_handler;
};

class application:public QApplication
{
public:
	template<class ... Args>
	explicit application(Args&&... args):
		QApplication{std::forward<Args>(args)...},
		m_internal_event_type{static_cast<QEvent::Type>(QEvent::registerEventType())}
	{}

	struct internal_event:public QEvent
	{
		std::function<bool()> callback;

		template<class Callable>
	  explicit internal_event(QEvent::Type event_type, Callable&& f) :
			QEvent{event_type},
			callback{std::forward<Callable>(f)}
		{}

		auto fire() const
		{ return callback(); }
	};

	bool event(QEvent* e) override
	{
		if(e->type() == m_internal_event_type)
		{ return static_cast<internal_event*>(e)->fire(); }
		return QApplication::event(e);
	}

	template<class Callable>
	void post_event(Callable&& callback)
	{
		// NOTE: We use naked new here because Qt will take ownership of the object
		postEvent(this, new internal_event{m_internal_event_type, std::forward<Callable>(callback)});
	}

private:
	QEvent::Type m_internal_event_type;
};

struct fdcb_writer
{
	std::reference_wrapper<application> app;
	std::reference_wrapper<QTextEdit> console;
};

size_t write(fdcb_writer writer, std::span<std::byte const> buffer)
{
	writer.app.get().post_event([&console = writer.console.get(), buffer](){
		std::string str{};
		str.reserve(std::size(buffer));
		std::ranges::transform(buffer, std::back_inserter(str), [](auto src){
			return static_cast<char>(src);
		});
		console.moveCursor(QTextCursor::End);
		console.insertPlainText(QString::fromStdString(str));
		console.moveCursor(QTextCursor::End);
		return true;
	});
	return std::size(buffer);
}

int main(int argc, char** argv)
{
	application my_app{argc, argv};
	QSplitter mainwin;
	mainwin.setOrientation(Qt::Vertical);

	QWidget top;
	mainwin.addWidget(&top);
	qt_form my_form{&top};

	QWidget bottom;
	mainwin.addWidget(&bottom);
	QTextEdit console_text{};
	QBoxLayout console_layout{QBoxLayout::Direction::TopToBottom,&bottom};
	console_layout.addWidget(&console_text);

	domain_size dom{
		.width = 49152,
		.height = 49152,
		.number_of_pixels = 1024*1024
	};
	bind(my_form, dom);
	my_form.refresh();
	my_form.set_focus();
	mainwin.show();

	fdcb::context stderr_redirect{STDERR_FILENO, fdcb_writer{my_app, console_text}};

	my_app.exec();
}