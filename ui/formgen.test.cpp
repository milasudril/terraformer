//@	{
//@		"target":{
//@			"name":"formgen_test",
//@			"dependencies":[
//				{"ref":"Qt5Widgets", "origin":"pkg-config"}
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

#include "./application_log.hpp"
#include "./application.hpp"
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

int main(int argc, char** argv)
{
	terraformer::application my_app{argc, argv};
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

	fdcb::context stderr_redirect{
		STDERR_FILENO,
		terraformer::application_log{
			.app = my_app,
			.console = console_text
		}
	};

	my_app.exec();
}