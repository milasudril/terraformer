#ifndef TERRAFORMER_MAINRIDGE_DESCRIPTION_HPP
#define TERRAFORMER_MAINRIDGE_DESCRIPTION_HPP

#include <functional>

namespace terraformer
{
	struct main_ridge_description
	{
		float y0;
		float z0;
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
					.binding = std::ref(params.get().y0)
				}
			}
		);

		form.insert(
			field{
				.name = "z0",
				.display_name = "z<sub>0</sub>",
				.description = "Controls the nominal elevation at the main ridge",
				.widget = textbox{
					.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = -std::numeric_limits<float>::infinity(),
							.max = std::numeric_limits<float>::infinity()
						}
					},
					.binding = std::ref(params.get().z0)
				}
			}
		);
	}
}


#endif