#ifndef TERRAFORMER_PARAMETERS_HPP
#define TERRAFORMER_PARAMETERS_HPP

#include "./domain_size.hpp"

namespace terraformer
{
	inline int dummy;

	struct simulation
	{
		struct domain_size domain_size;
	};

	template<class Form>
	void bind(Form& form, simulation& params)
	{
		form.insert(
			field{
				.name = "domain_size",
				.display_name = "Domain size",
				.description = "Sets the size of the domain",
				.widget = subform{
					.binding = std::ref(params.domain_size)
				}
			}
		);

		form.insert(field{
			.name = "foo",
			.display_name ="bar",
			.description ="kaka",
			.widget = textbox{
				.value_converter = string_converter{
					.range = open_open_interval{
						.min = 0,
						.max = 10
					}
				},
				.binding = std::ref(dummy)
			}
		});
	}
}

#endif