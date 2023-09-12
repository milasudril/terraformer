//@	{
//@		"target":{
//@			"name":"terraformer-gui.o",
//@			"dependencies":[
//@				{"ref":"fdcb", "rel":"external"}
//@			]
//@		},
//	NOTE: We need to change some compiler flags for Qt
//@		"compiler":{
//@			"config": {
//@				"cflags":[
//	Qt headers uses some ugly implicit conversions
//@					"-Wno-error=conversion",
//	Qt headers uses enum conversions that have been deprecated in C++20
//@					"-Wno-error=deprecated-enum-enum-conversion",
//	Qt requires that we build with fpic
//@					"-fpic"
//@				]
//@			}
//@		}
//@	}

#include "ui/application_log.hpp"
#include "ui/application.hpp"
#include "ui/form.hpp"
#include "lib/modules/simulation.hpp"

#include <QSplitter>
#include <fdcb.h>

int main(int argc, char** argv)
{
	terraformer::application terraformer{argc, argv};
	QSplitter mainwin;
	mainwin.setOrientation(Qt::Vertical);

	terraformer::form my_form{nullptr};
	mainwin.addWidget(&my_form);

	QWidget bottom;
	mainwin.addWidget(&bottom);
	QTextEdit console_text{};
	QBoxLayout console_layout{QBoxLayout::Direction::TopToBottom,&bottom};
	console_layout.addWidget(&console_text);

	terraformer::simulation sim{
		.domain_size{
			.width = 49152,
			.height = 49152,
			.number_of_pixels = 1024*1024
		}
	};
	bind(my_form, sim);
	my_form.setObjectName("simulation");
	my_form.refresh();
	mainwin.show();
/*
	fdcb::context stderr_redirect{
		STDERR_FILENO,
		terraformer::application_log{
			.app = terraformer,
			.console = console_text
		}
	};*/

	return terraformer.exec();
}