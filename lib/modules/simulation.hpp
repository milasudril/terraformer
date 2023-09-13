#ifndef TERRAFORMER_PARAMETERS_HPP
#define TERRAFORMER_PARAMETERS_HPP

#include "./domain_size.hpp"

#include <pcg-cpp/include/pcg_random.hpp>

namespace terraformer
{
	using random_generator = pcg_engines::oneseq_dxsm_128_64;
	using rng_seed_type = __int128;

	struct simulation
	{
		struct domain_size domain_size;
		rng_seed_type rng_seed;
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
			.name = "rng_seed",
			.display_name ="Random generator seed",
			.description = "Sets the initial value for the random bit generator",
			.widget = textbox{
				.value_converter = hash_string_converter<rng_seed_type>{},
				.binding = std::ref(params.rng_seed)
			}
		});
	}
}

#endif