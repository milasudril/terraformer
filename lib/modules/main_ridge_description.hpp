#ifndef TERRAFORMER_MAINRIDGE_DESCRIPTION_HPP
#define TERRAFORMER_MAINRIDGE_DESCRIPTION_HPP

#include "./fractal_wave.hpp"

#include <functional>

namespace terraformer
{
	template<class OffsetType, class AmplitudeType>
	struct ridge_curve_description
	{
		OffsetType initial_value;
		AmplitudeType amplitude;
		fractal_wave_description wave;
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
				.widget = textbox{
					.value_converter = num_string_converter<float>{},
					.binding = std::ref(params.get().initial_value)
				}
			}
		);

		form.insert(
			field{
				.name = "amplitude",
				.display_name = "Amplitude",
				.description = "Sets the amplitude of the generated wave function",
				.widget = textbox{
					.value_converter = num_string_converter<float>{},
					.binding = std::ref(params.get().amplitude)
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