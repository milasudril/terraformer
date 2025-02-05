#ifndef TERRAFORMER_INITIAL_HEIGHTMAP_DESCRIPTION_HPP
#define TERRAFORMER_INITIAL_HEIGHTMAP_DESCRIPTION_HPP

#include "./main_ridge_description.hpp"
#include "./calculator.hpp"

namespace terraformer
{
	enum elevation_range_control_mode
	{
		use_guides_only,
		normalize_output,
		softclamp_output,
		clamp_output
	};

	struct elevation_range_control
	{
		elevation min{elevation::default_value()/1.25f};
		elevation max{elevation::default_value()*1.25f};
		elevation_range_control_mode control_mode;
	};

	struct elevation_range_control_mode_converter
	{
		static constexpr int convert(elevation_range_control_mode value)
		{
			switch(value)
			{
				case elevation_range_control_mode::use_guides_only:
					return 0;
				case elevation_range_control_mode::normalize_output:
					return 1;
				case elevation_range_control_mode::softclamp_output:
					return 2;
				case elevation_range_control_mode::clamp_output:
					return 3;
			}
			__builtin_unreachable();
		}

		static constexpr elevation_range_control_mode convert(int value)
		{
			switch(value)
			{
				case 0:
					return elevation_range_control_mode::use_guides_only;
				case 1:
					return elevation_range_control_mode::normalize_output;
				case 2:
					return elevation_range_control_mode::softclamp_output;
				case 3:
					return elevation_range_control_mode::clamp_output;
				default:
					throw std::runtime_error{"Invalid elevation range control mode"};
			}
		}
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, elevation_range_control>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "min",
			.display_name = "Min",
			.description = "Sets the minimum elevation",
			.widget = numeric_input{
				.binding = std::ref(params.get().min),
				.value_converter = calculator{}
			},
		});

		form.insert(field{
			.name = "max",
			.display_name = "Max",
			.description = "Sets the maximum elevation",
			.widget = numeric_input{
				.binding = std::ref(params.get().max),
				.value_converter = calculator{}
			},
		});

		form.insert(field{
			.name = "mode",
			.display_name = "Control mode",
			.description = "Selects the elevation range control mode",
			.widget = enum_input{
				.binding = std::ref(params.get().control_mode),
				.value_converter = elevation_range_control_mode_converter{},
				.labels = std::array<char const*, 4>{
					"Use guides only",
					"Normalize output",
					"Softclamp output",
					"Clamp output"
				}
			}
		});
	}

	struct surface_control_point
	{
		elevation z;
		slope_angle slope_x;
		slope_angle slope_y;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, surface_control_point>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "z",
				.display_name = "Elevation",
				.description = "Sets the elevation of the control point",
				.widget = numeric_input{
					.binding = std::ref(params.get().z),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "slope_x",
				.display_name = "Slope X",
				.description = "Sets the elevation angle in the x direction",
				.widget = numeric_input{
					.binding = std::ref(params.get().slope_x),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "slope_y",
				.display_name = "Slope Y",
				.description = "Sets the elevation angle in the y direction",
				.widget = numeric_input{
					.binding = std::ref(params.get().slope_y),
					.value_converter = calculator{}
				}
			}
		);
	}

	struct corners
	{
		surface_control_point nw;
		surface_control_point ne;
		surface_control_point sw;
		surface_control_point se;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, corners>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "nw",
				.display_name = "NW",
				.description = "Controls the north-west corner",
				.widget = subform{
					.binding = std::ref(params.get().nw),
				}
			}
		);

		form.insert(
			field{
				.name = "ne",
				.display_name = "NE",
				.description = "Controls the north-east corner",
				.widget = subform{
					.binding = std::ref(params.get().ne),
				},
			}
		);

		form.insert(
			field{
				.name = "sw",
				.display_name = "SW",
				.description = "Controls the south-west corner",
				.widget = subform{
					.binding = std::ref(params.get().sw),
				}
			}
		);

		form.insert(
			field{
				.name = "se",
				.display_name = "SE",
				.description = "Controls the south-east corner",
				.widget = subform{
					.binding = std::ref(params.get().se),
				},
			}
		);
	}

	struct damped_wave_description
	{
		vertical_amplitude initial_amplitude;
		horizontal_offset peak_location;
		bool flip_direction{};
		bool invert_displacement{};
		domain_length half_distance;
		filtered_noise_1d_description wave;
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
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "peak_location",
				.display_name = "Peak location",
				.description = "Sets the peak location of the modulated wave",
				.widget = numeric_input{
					.binding = std::ref(params.get().peak_location),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "flip_direction",
				.display_name = "Flip direction",
				.description = "Flips the \"travel direction\" of the wave",
				.widget = bool_input{
					.binding = std::ref(params.get().flip_direction)
				}
			}
		);

		form.insert(
			field{
				.name = "invert_displacement",
				.display_name = "Invert displacement",
				.description = "Inverts displacements caused by the wave the wave",
				.widget = bool_input{
					.binding = std::ref(params.get().invert_displacement)
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
					.value_converter = calculator{}
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
		horizontal_offset peak_location;
		bool flip_direction{};
		bool invert_displacement{};
		filtered_noise_1d_description modulating_wave;
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
					.value_converter = calculator{}
				}
		});

		form.insert(
			field{
				.name = "peak_location",
				.display_name = "Peak location",
				.description = "Sets the peak location of the modulating wave",
				.widget = numeric_input{
					.binding = std::ref(params.get().peak_location),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "flip_direction",
				.display_name = "Flip direction",
				.description = "Flips the \"travel direction\" of the wave",
				.widget = bool_input{
					.binding = std::ref(params.get().flip_direction)
				}
			}
		);

		form.insert(
			field{
				.name = "invert_displacement",
				.display_name = "Invert displacement",
				.description = "Inverts displacements caused by the wave the wave",
				.widget = bool_input{
					.binding = std::ref(params.get().invert_displacement)
				}
			}
		);

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
				.name = "half_distance_modulation",
				.display_name = "Half distance modulation",
				.description = "Controls half distance modulation",
				.widget = subform{
					.binding = std::ref(params.get().half_distance_modulation)
				}
			}
		);
	}

	struct bump_field_description
	{
		vertical_amplitude amplitude;
		spline_gradient valley_gradient;
		spline_gradient peak_gradient;
		horizontal_offset peak_loc_x;
		horizontal_offset peak_loc_y;
		opening_angle axis_angle;
		rotation_angle rotation;
		filtered_noise_2d_description wave;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, bump_field_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "amplitude",
				.display_name = "Amplitude",
				.description = "Sets the amplitude of the generated wave function",
				.widget = numeric_input_log{
					.binding = std::ref(params.get().amplitude),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "valley_gradient",
				.display_name = "Valley gradient",
				.description = "Sets the gradient used for valleys",
				.widget = numeric_input_log{
					.binding = std::ref(params.get().valley_gradient),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "peak_gradient",
				.display_name = "Peak gradient",
				.description = "Sets the gradient used for peak",
				.widget = numeric_input_log{
					.binding = std::ref(params.get().peak_gradient),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "peak_loc_x",
				.display_name = "Peak location X",
				.description = "Sets X coordinate of the highest point",
				.widget = numeric_input{
					.binding = std::ref(params.get().peak_loc_x),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "peak_loc_y",
				.display_name = "Peak location Y",
				.description = "Sets Y coordinate of the highest point",
				.widget = numeric_input{
					.binding = std::ref(params.get().peak_loc_y),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "axis_angle",
				.display_name = "Axis angle",
				.description = "Sets the angle between X and Y axis",
				.widget = numeric_input{
					.binding = std::ref(params.get().axis_angle),
					.value_converter = calculator{}
				}
			}
		);

		form.insert(
			field{
				.name = "rotation",
				.display_name = "Rotation",
				.description = "Sets the rotation of the coordinate system",
				.widget = numeric_input{
					.binding = std::ref(params.get().rotation),
					.value_converter = calculator{}
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
		elevation_range_control output_range;
		struct corners corners;
		main_ridge_description main_ridge;
		damped_wave_description ns_distortion;
		bump_field_description bump_field;
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
				.display_name = "Corners",
				.description = "Controls the elevation and slopes at the corners",
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
				.name = "ns_distortion",
				.display_name = "North-south distortion",
				.description = "Controls properties north-south distortion",
				.widget = subform{
					.binding = std::ref(params.get().ns_distortion)
				}
			}
		);

		form.insert(
			field{
				.name = "bump_field",
				.display_name = "Bumpfield",
				.description = "Controls properties the bump field",
				.widget = subform{
					.binding = std::ref(params.get().bump_field)
				}
			}
		);
	}
}

#endif
