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

#include "./form.hpp"
#include "lib/modules/domain_size.hpp"
#include "lib/common/utils.hpp"

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
	terraformer::form my_form{&top};

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