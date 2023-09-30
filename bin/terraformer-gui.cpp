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
#include "lib/modules/simulation_description.hpp"
#include "lib/modules/heightmap.hpp"
#include "lib/common/random_bit_source.hpp"
#include "lib/execution/task_receiver.hpp"

#include <QSplitter>
#include <QScrollArea>
#include <fdcb.h>

int main(int argc, char** argv)
{
	terraformer::application terraformer{argc, argv};
	terraformer.setStyleSheet("*{padding:0px; margin:0px}\nQPushButton{padding:4px}");
	QSplitter mainwin;
	mainwin.setOrientation(Qt::Vertical);

	QSplitter input_output{nullptr};
	mainwin.addWidget(&input_output);

	terraformer::simulation_description sim{
		.rng_seed = terraformer::random_bit_source{}.get<terraformer::rng_seed_type>(),
		.domain_size{
			.width = 49152,
			.height = 49152,
			.number_of_pixels = 1024*1024
		},
		.initial_heightmap{
			.corners{
				.nw = 4096.0f,
				.ne = 3072.0f,
				.sw = 512.0f,
				.se = 1536.0f,
			},
			.main_ridge{
				.ridge_curve_xy{
					.initial_value = 1.0f/3.0f,
					.amplitude = 4096.0f/49152.0f,
					.wave{
						.shape{
							.amplitude{
								.scaling_factor = 2.0f,
								.scaling_noise = 1.0f/16.0f
							},
							.wavelength{
								.scaling_factor = std::numbers::phi_v<float>,
								.scaling_noise = std::numbers::phi_v<float>/16.0f
							},
							.phase{
								.offset = 2.0f - std::numbers::phi_v<float>,
								.offset_noise = 1.0f/16.0f
							}
						},
						.wave_properties{
							.wavelength = 24576.0f,
							.phase = 0.0f
						}
					}
				},
				.ridge_curve_xz{
					.initial_value = 8192.0f,
					.amplitude = 512.0f,
					.wave{
						.shape{
							.amplitude{
								.scaling_factor = std::numbers::phi_v<float>,
								.scaling_noise = 1.0f/16.0f
							},
							.wavelength{
								.scaling_factor = std::numbers::phi_v<float>,
								.scaling_noise = std::numbers::phi_v<float>/16.0f
							},
							.phase{
								.offset = 2.0f - std::numbers::phi_v<float>,
								.offset_noise = 1.0f/16.0f
							}
						},
						.wave_properties{
							.wavelength = 12384.0f,
							.phase = 0.0f
						}
					}
				}
			}
		}
	};

	auto initial_heightmap = make_heightmap(sim.domain_size);
	terraformer::random_generator rng{sim.rng_seed};
	generate(initial_heightmap, sim.initial_heightmap, rng);

	terraformer::form output{nullptr, "result", [](auto&&...){}};

	terraformer::task_receiver<std::function<void()>> processor;

	terraformer::form input{nullptr, "simulation_description", [
			&terraformer,
			&output,
			&initial_heightmap,
			&sim = std::as_const(sim),
			&processor](auto&& field_name) {
		processor.replace_pending_task([
				sim,
				field_name = std::move(field_name),
				&terraformer,
				&output,
				&initial_heightmap](){
			fprintf(stderr, "(i) %s was changed\n", field_name.c_str());
			terraformer::random_generator rng{sim.rng_seed};
			auto new_heightmap = make_heightmap(sim.domain_size);
			generate(new_heightmap, sim.initial_heightmap, rng);
			terraformer.post_event([&output, &initial_heightmap, res = std::move(new_heightmap)]() mutable {
				initial_heightmap = std::move(res);
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

	input.set_focus();
	input.refresh();
	mainwin.show();
	output.refresh();
	input_scroll_area.setMinimumWidth(384);
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
