#ifndef TERRAFORMER_MAINRIDGE_DESCRIPTION_HPP
#define TERRAFORMER_MAINRIDGE_DESCRIPTION_HPP

#include "./fractal_wave.hpp"
#include "./calculator.hpp"
#include "./filtered_noise_generator.hpp"

#include <functional>

namespace terraformer
{
	template<class OffsetType, class AmplitudeType>
	struct ridge_curve_description
	{
		OffsetType initial_value;
		AmplitudeType amplitude;
		horizontal_offset peak_location;
		bool flip_x{};
		bool flip_displacement{};
		filtered_noise_description_1d wave;
	};

	template<class Form, class OffsetType, class AmplitudeType>
	void bind(Form& form,
		std::reference_wrapper<ridge_curve_description<OffsetType, AmplitudeType>> params)
	{
		form.insert(
			field{
				.name = "initial_value",
				.display_name = "Initial value",
				.description = "Sets the initial value of the generated wave function",
				.widget = numeric_input{
					.binding = std::ref(params.get().initial_value),
					.value_converter = calculator{}
				}
			}
		);

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
				.name = "flip_x",
				.display_name = "Flip x",
				.description = "Filps the x direction",
				.widget = bool_input{
					.binding = std::ref(params.get().flip_x)
				}
			}
		);

		form.insert(
			field{
				.name = "flip_displacement",
				.display_name = "Flip displacement",
				.description = "Filps the displacement direction (y or z)",
				.widget = bool_input{
					.binding = std::ref(params.get().flip_displacement)
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

	struct main_ridge_description
	{
		ridge_curve_description<domain_length, horizontal_amplitude> ridge_curve_xy;
		ridge_curve_description<elevation, vertical_amplitude> ridge_curve_xz;
	};

	inline std::vector<location> generate(main_ridge_description const& params,
		random_generator& rng,
		uint32_t num_pixels,
		float dx)
	{
		return generate(rng,
			params.ridge_curve_xy.wave,
			params.ridge_curve_xy.amplitude,
			params.ridge_curve_xy.peak_location,
			params.ridge_curve_xy.flip_x,
			params.ridge_curve_xy.flip_displacement,
			params.ridge_curve_xz.wave,
			params.ridge_curve_xz.amplitude,
			params.ridge_curve_xz.peak_location,
			params.ridge_curve_xz.flip_x,
			params.ridge_curve_xz.flip_displacement,
			polyline_location_params{
				.point_count = num_pixels,
				.dx = dx,
				.start_location = terraformer::location{
					0.0f,
					static_cast<float>(params.ridge_curve_xy.initial_value),
					static_cast<float>(params.ridge_curve_xz.initial_value)
				}
			}
		);
	}

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, main_ridge_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "ridge_curve_xy",
				.display_name = "Horizontal ridge curve",
				.description = "Controls the horizontal properties of the main ridge",
				.widget = subform{
					.binding = std::ref(params.get().ridge_curve_xy)
				}
			}
		);

		form.insert(
			field{
				.name = "ridge_curve_xz",
				.display_name = "Vertical ridge curve",
				.description = "Controls the vertical properties of the main ridge",
				.widget = subform{
					.binding = std::ref(params.get().ridge_curve_xz)
				}
			}
		);
	}
}
#endif