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

#include "lib/modules/domain_size.hpp"

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
	std::unique_ptr<QWidget> create_widget(terraformer::textbox<Converter, BindingType> const& textbox)
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
					textbox.binding.get() = textbox.value_converter.convert(str);
				}, src, textbox);
				refresh();
			}
		);
		m_display_callbacks.push_back([&dest = *ret, textbox](){
			dest.setText(textbox.value_converter.convert(textbox.binding.get()).c_str());
		});

		return ret;
	}

	template<class Converter, class BindingType>
	std::unique_ptr<QWidget> create_widget(terraformer::text_display<Converter, BindingType>&& text_display)
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

	terraformer::domain_size dom{
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