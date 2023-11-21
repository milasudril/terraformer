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
//	QtCharts does not use override everywhere where it is applicable
//@					"-Wno-error=suggest-override",
//	Qt requires that we build with fpic
//@					"-fpic"
//@				]
//@			}
//@		}
//@	}

#include "ui/application_log.hpp"
#include "ui/application.hpp"
#include "ui/form.hpp"
#include "lib/modules/simulation_description.hpp"
#include "lib/modules/heightmap.hpp"
#include "lib/execution/task_receiver.hpp"

#include <QSplitter>
#include <QScrollArea>
#include <fdcb.h>

int main(int argc, char** argv)
{
	terraformer::simulation_description sim{};
	terraformer::random_generator rng{sim.rng_seed};
	terraformer::heightmap initial_heightmap{
		make_domain_resolution(sim.domain_size),
		sim.initial_heightmap,
		rng
	};

	terraformer::application terraformer{argc, argv};
	terraformer.setStyleSheet("*{padding:0px; margin:0px}\nQPushButton{padding:4px}");
	QSplitter mainwin;
	mainwin.setOrientation(Qt::Vertical);

	QSplitter input_output{nullptr};
	mainwin.addWidget(&input_output);

	terraformer::form output{nullptr, "result", [](auto&&...){}};

	auto temp_heightmap = initial_heightmap;
	terraformer::task_receiver<std::function<void()>> processor;
	terraformer::form input{nullptr, "simulation_description", [
			&terraformer,
			&output,
			&initial_heightmap,
			&temp_heightmap,
			&sim = std::as_const(sim),
			&processor](auto&& field_name) {
		processor.replace_pending_task([
				sim,
				field_name = std::move(field_name),
				&terraformer,
				&output,
				&initial_heightmap,
				&temp_heightmap](){
			terraformer::random_generator rng{sim.rng_seed};
			if(field_name == "simulation_description/rng_seed")
			{
				temp_heightmap.rng_seed_updated(sim.initial_heightmap, rng);
				goto done;
			}

			if(field_name.starts_with("simulation_description/domain_size/"))
			{
				temp_heightmap.domain_size_updated(sim.domain_size, sim.initial_heightmap, rng);
				goto done;
			}

			if(field_name.starts_with("simulation_description/initial_heightmap/output_range/"))
			{
				temp_heightmap.output_range_updated(sim.initial_heightmap, rng);
				goto done;
			}

			if(field_name.starts_with("simulation_description/initial_heightmap/corners/"))
			{
				temp_heightmap.corners_updated(sim.initial_heightmap, rng);
				goto done;
			}

			if(field_name.starts_with("simulation_description/initial_heightmap/main_ridge/"))
			{
				temp_heightmap.main_ridge_updated(sim.initial_heightmap, rng);
				goto done;
			}

			if(field_name.starts_with("simulation_description/initial_heightmap/ns_distortion/"))
			{
				temp_heightmap.ns_distortion_updated(sim.initial_heightmap, rng);
				goto done;
			}

			if(field_name.starts_with("simulation_description/initial_heightmap/bump_field/"))
			{
				temp_heightmap.bump_field_updated(sim.initial_heightmap, rng);
				goto done;
			}

			fprintf(stderr, "(x) %s was changed\n", field_name.c_str());
			done:
			terraformer.post_event([&output, &initial_heightmap, temp_heightmap]() mutable {
				initial_heightmap = std::move(temp_heightmap);
				output.refresh();
				return true;
			});
		});
	}};
	input.setObjectName("simulation_description");

	QWidget bottom;
	mainwin.addWidget(&bottom);
	QTextEdit console_text{};
	console_text.setReadOnly(true);
	QBoxLayout console_layout{QBoxLayout::Direction::TopToBottom,&bottom};
	console_layout.addWidget(&console_text);

	bind(input, std::ref(sim));
	bind(output, std::cref(initial_heightmap));

	QScrollArea input_scroll_area{&input_output};
	input_scroll_area.setWidget(&input);
	input_output.addWidget(&output);
	input_scroll_area.setWidgetResizable(true);

	input.set_focus();
	input.refresh();
	mainwin.show();
	output.refresh();
	input_scroll_area.setMinimumWidth(352);
	input_scroll_area.adjustSize();

	fdcb::context stderr_redirect{
		STDERR_FILENO,
		terraformer::application_log{
			.app = terraformer,
			.console = console_text
		}
	};

	auto ret = terraformer.exec();
	// NOTE: QScrollArea is odd because it takes ownership of the widget. Fix this issue
	//       by calling takeWidget
	(void)input_scroll_area.takeWidget();
	return ret;
}
