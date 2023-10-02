#ifndef TERRAFORMER_INITIAL_HEIGHTMAP_DESCRIPTION_HPP
#define TERRAFORMER_INITIAL_HEIGHTMAP_DESCRIPTION_HPP

#include "./main_ridge_description.hpp"

namespace terraformer
{
	struct elevation_range
	{
		float min;
		float max;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, elevation_range>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "min",
			.display_name = "Min",
			.description = "Sets the minimum elevation",
			.widget = textbox{
				.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = -std::numeric_limits<float>::infinity(),
							.max = std::numeric_limits<float>::infinity()
						}
					},
				.binding = std::ref(params.get().min)
			},
		});

		form.insert(field{
			.name = "max",
			.display_name = "Max",
			.description = "Sets the maximum elevation",
			.widget = textbox{
				.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = -std::numeric_limits<float>::infinity(),
							.max = std::numeric_limits<float>::infinity()
						}
					},
				.binding = std::ref(params.get().max)
			},
		});
	}

	struct corner
	{
		float elevation;
	};

	struct corners
	{
		corner nw;
		corner ne;
		corner sw;
		corner se;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, corners>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "nw_elev",
			.display_name = "NW",
			.description = "Sets the elevation in north-west",
			.widget = textbox{
				.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = -std::numeric_limits<float>::infinity(),
							.max = std::numeric_limits<float>::infinity()
						}
					},
				.binding = std::ref(params.get().nw.elevation)
			},
		});

		form.insert(field{
			.name = "ne_elev",
			.display_name = "NE",
			.description = "Sets the elevation in north-east",
			.widget = textbox{
				.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = -std::numeric_limits<float>::infinity(),
							.max = std::numeric_limits<float>::infinity()
						}
					},
				.binding = std::ref(params.get().ne.elevation)
			},
		});

		form.insert(field{
			.name = "sw_elev",
			.display_name = "SW",
			.description = "Sets the elevation in south-west",
			.widget = textbox{
				.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = -std::numeric_limits<float>::infinity(),
							.max = std::numeric_limits<float>::infinity()
						}
					},
				.binding = std::ref(params.get().sw.elevation)
			},
		});

		form.insert(field{
			.name = "se_elev",
			.display_name = "SE",
			.description = "Sets the elevation in south-east",
			.widget = textbox{
				.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = -std::numeric_limits<float>::infinity(),
							.max = std::numeric_limits<float>::infinity()
						}
					},
				.binding = std::ref(params.get().se.elevation)
			},
		});
	}

	struct plane_wave_description
	{
		float amplitude;
		fractal_wave_description wave;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, plane_wave_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "amplitude",
				.display_name = "Amplitude",
				.description = "Sets the amplitude of the generated wave function",
				.widget = std::tuple{
					knob{
						.min = -1.0f,
						.max = 15.0f,
						.binding = std::ref(params.get().amplitude),
						.mapping = numeric_input_mapping_type::log
					},
					textbox{
						.value_converter = num_string_converter{
							.range = closed_open_interval{
								.min = 0.0f,
								.max = std::numeric_limits<float>::infinity()
							}
						},
						.binding = std::ref(params.get().amplitude)
					}
				}
			}
		);
		form.insert(
			field{
				.name = "wave",
				.display_name = "Wave parameters",
				.description = "Sets different parameters for the wave function",
				.widget = subform{
					.binding = std::ref(params.get().wave)
				}
			}
		);
	}

	struct initial_heightmap_description
	{
		elevation_range output_range;
		struct corners corners;
		main_ridge_description main_ridge;
		plane_wave_description ns_wave;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, initial_heightmap_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "output_range",
				.display_name = "Output range",
				.description = "Sets the output elevation range. Data is normalized to fit within these limits",
				.widget = subform{
					.binding = std::ref(params.get().output_range)
				}
			}
		);

		form.insert(
			field{
				.name = "corners",
				.display_name = "Corner elevations",
				.description = "Sets elevation at domain corners",
				.widget = subform{
					.binding = std::ref(params.get().corners)
				}
			}
		);

		form.insert(
			field{
				.name = "main_ridge",
				.display_name = "Main ridge parameters",
				.description = "Controls the location and shape of the main ridge",
				.widget = subform{
					.binding = std::ref(params.get().main_ridge)
				}
			}
		);

		form.insert(
			field{
				.name = "ns_wave",
				.display_name = "North-south wave",
				.description = "Controls properites of the north-south wave",
				.widget = subform{
					.binding = std::ref(params.get().ns_wave)
				}
			}
		);
	}
}

#endif
