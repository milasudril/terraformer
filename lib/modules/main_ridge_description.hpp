#ifndef TERRAFORMER_MAINRIDGE_DESCRIPTION_HPP
#define TERRAFORMER_MAINRIDGE_DESCRIPTION_HPP

#include <functional>

namespace terraformer
{
	struct main_ridge_description
	{
		float y_0;
//		terraformer::fractal_wave_params ridge_curve;
//		terraformer::fractal_wave_params ridge_curve_xz;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, main_ridge_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(
			field{
				.name = "y0",
				.display_name = "y<sub>0</sub>",
				.description = "Controls the main ridge offset in the north-south direction",
				.widget = textbox{
					.value_converter = num_string_converter{
						.range = closed_closed_interval{
							.min = 0.0f,
							.max = 1.0f
						}
					},
					.binding = std::ref(params.get().y_0)
				}
			}
		);
	}
}


#endif