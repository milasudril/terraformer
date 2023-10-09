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

	struct damped_wave_description
	{
		float half_distance;
		fractal_wave_description wave;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, damped_wave_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "half_distance",
				.display_name = "Half distance",
				.description = "The disntance that halfs the amplitude",
				.widget = std::tuple{
					knob{
						.min = 8.0f,
						.max = 17.0f,
						.binding = std::ref(params.get().half_distance),
						.mapping = numeric_input_mapping_type::log
					},
					textbox{
						.value_converter = num_string_converter{
							.range = closed_open_interval{
								.min = 256.0f,
								.max = std::numeric_limits<float>::infinity()
							}
						},
						.binding = std::ref(params.get().half_distance)
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

	struct modulation_description
	{
		float depth;
		fractal_wave_description modulating_wave;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, modulation_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "depth",
			.display_name = "Depth",
			.description = "Sets the modulation depth",
			.widget = std::tuple{
				knob{
					.min = 0.0f,
					.max = 1.0f,
					.binding = std::ref(params.get().depth),
					.mapping = numeric_input_mapping_type::lin
				},
				textbox{
					.value_converter = num_string_converter{
						.range = closed_closed_interval{
							.min = 0.0f,
							.max = 1.0f,
						}
					},
					.binding = std::ref(params.get().depth)
				}
			}
		});

		form.insert(field{
			.name = "modulating_wave",
			.display_name = "Modulating wave",
			.description = "Controls the shape of the modulatiing wave",
			.widget = subform{
				.binding = std::ref(params.get().modulating_wave)
			}
		});
	}

	struct modulated_damped_wave_description
	{
		float initial_amplitude;
		damped_wave_description wave;
		modulation_description amplitude_modulation;
		modulation_description wavelength_modulation;
		modulation_description half_distance_modulation;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, modulated_damped_wave_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "initial_amplitude",
				.display_name = "Initial amplitude",
				.description = "Initial (undamped) amplitude of the generated wave",
				.widget = std::tuple{
					knob{
						.min = -1.0f,
						.max = 15.0f,
						.binding = std::ref(params.get().initial_amplitude),
						.mapping = numeric_input_mapping_type::log
					},
					textbox{
						.value_converter = num_string_converter{
							.range = closed_open_interval{
								.min = 0.0f,
								.max = std::numeric_limits<float>::infinity()
							}
						},
						.binding = std::ref(params.get().initial_amplitude)
					}
				}
			}
		);

		form.insert(
			field{
				.name = "wave",
				.display_name = "Wave",
				.description = "Controls the behaviour of the wave",
				.widget = subform{
					.binding = std::ref(params.get().wave)
				}
			}
		);

		form.insert(
			field{
				.name = "amplitude_modulation",
				.display_name = "Amplitude modulation",
				.description = "Controls amplitude modulation",
				.widget = subform{
					.binding = std::ref(params.get().amplitude_modulation)
				}
			}
		);

		form.insert(
			field{
				.name = "wavelength_modulation",
				.display_name = "Wavelength modulation",
				.description = "Controls wavelength modulation",
				.widget = subform{
					.binding = std::ref(params.get().wavelength_modulation)
				}
			}
		);

		form.insert(
			field{
				.name = "half_distance_modulation",
				.display_name = "Half distance modulation",
				.description = "Controls half distance modulation",
				.widget = subform{
					.binding = std::ref(params.get().half_distance_modulation)
				}
			}
		);
	}

	struct initial_heightmap_description
	{
		elevation_range output_range;
		struct corners corners;
		main_ridge_description main_ridge;
		modulated_damped_wave_description ns_wave;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, initial_heightmap_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
#if 0
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
#endif

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
