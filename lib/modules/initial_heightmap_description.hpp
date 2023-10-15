#ifndef TERRAFORMER_INITIAL_HEIGHTMAP_DESCRIPTION_HPP
#define TERRAFORMER_INITIAL_HEIGHTMAP_DESCRIPTION_HPP

#include "./main_ridge_description.hpp"

namespace terraformer
{
	struct elevation_range
	{
		elevation min;
		elevation max;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, elevation_range>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "min",
			.display_name = "Min",
			.description = "Sets the minimum elevation",
			.widget = numeric_input{
				.binding = std::ref(params.get().min),
				.value_converter = num_string_converter<float>{}
			},
		});

		form.insert(field{
			.name = "max",
			.display_name = "Max",
			.description = "Sets the maximum elevation",
			.widget = numeric_input{
				.binding = std::ref(params.get().max),
				.value_converter = num_string_converter<float>{}
			},
		});
	}

	struct corner
	{
		elevation z;
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
			.widget = numeric_input{
				.binding = std::ref(params.get().nw.z),
				.value_converter = num_string_converter<float>{}
			},
		});

		form.insert(field{
			.name = "ne_elev",
			.display_name = "NE",
			.description = "Sets the elevation in north-east",
			.widget = numeric_input{
				.binding = std::ref(params.get().ne.z),
				.value_converter = num_string_converter<float>{}
			},
		});

		form.insert(field{
			.name = "sw_elev",
			.display_name = "SW",
			.description = "Sets the elevation in south-west",
			.widget = numeric_input{
				.binding = std::ref(params.get().sw.z),
				.value_converter = num_string_converter<float>{}
			},
		});

		form.insert(field{
			.name = "se_elev",
			.display_name = "SE",
			.description = "Sets the elevation in south-east",
			.widget = numeric_input{
				.binding = std::ref(params.get().se.z),
				.value_converter = num_string_converter<float>{},
			},
		});
	}

	struct damped_wave_description
	{
		vertical_amplitude initial_amplitude;
		domain_length half_distance;
		fractal_wave_description wave;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, damped_wave_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "initial_amplitude",
				.display_name = "Initial amplitude",
				.description = "Initial (undamped) amplitude of the generated wave",
				.widget = numeric_input_log{
					.binding = std::ref(params.get().initial_amplitude),
					.value_converter = num_string_converter<float>{}
				}
			}
		);

		form.insert(
			field{
				.name = "half_distance",
				.display_name = "Half distance",
				.description = "The disntance that halfs the amplitude",
				.widget = numeric_input_log{
					.binding = std::ref(params.get().half_distance),
					.value_converter = num_string_converter<float>{}
				}
			}
		);

		form.insert(
			field{
				.name = "wave_parameters",
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
		modulation_depth depth;
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
			.widget = numeric_input{
					.binding = std::ref(params.get().depth),
					.value_converter = num_string_converter<float>{}
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
		damped_wave_description nominal_oscillations;
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
				.name = "nominal_oscillations",
				.display_name = "Nominal oscillations",
				.description = "Controls the behaviour of oscillations without any modulation",
				.widget = subform{
					.binding = std::ref(params.get().nominal_oscillations)
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
		form.insert(
			field{
				.name = "output_range",
				.display_name = "Output range",
				.description = "Controls the output elevation range",
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
