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
#include "lib/common/random_bit_source.hpp"

#include <QSplitter>
#include <fdcb.h>

int main(int argc, char** argv)
{
	terraformer::application terraformer{argc, argv};
	terraformer.setStyleSheet("*{padding:0px; margin:0px}\nQPushButton{padding:4px}");
	QSplitter mainwin;
	mainwin.setOrientation(Qt::Vertical);
	
	QSplitter input_output{nullptr};
	mainwin.addWidget(&input_output);

	QLabel output;
	
	terraformer::form input{nullptr, "simulation", [&output](auto&& field_name) {
		fprintf(stderr, "(i) %s was changed\n", field_name.c_str());
		output.setText(QString::fromStdString(field_name));
	}};

	input_output.addWidget(&input);
	QBoxLayout output_layout{QBoxLayout::Direction::TopToBottom,&output};
	input_output.addWidget(&output);

	QWidget bottom;
	mainwin.addWidget(&bottom);
	QTextEdit console_text{};
	QBoxLayout console_layout{QBoxLayout::Direction::TopToBottom,&bottom};
	console_layout.addWidget(&console_text);

	terraformer::simulation sim{
		.rng_seed = terraformer::random_bit_source{}.get<terraformer::rng_seed_type>(),
		.domain_size{
			.width = 49152,
			.height = 49152,
			.number_of_pixels = 1024*1024
		},
		.initial_heightmap{}
	};

	bind(input, sim);
	input.setObjectName("simulation");
	input.refresh();
	input.set_focus();
	mainwin.show();

	fdcb::context stderr_redirect{
		STDERR_FILENO,
		terraformer::application_log{
			.app = terraformer,
			.console = console_text
		}
	};

	return terraformer.exec();
}