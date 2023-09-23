#ifndef TERRAFORMER_SIMULATIONDESCRIPTION_HPP
#define TERRAFORMER_SIMULATIONDESCRIPTION_HPP

#include "./domain_size_description.hpp"
#include "./initial_heightmap_description.hpp"
#include "lib/common/random_bit_source.hpp"

#include <pcg-cpp/include/pcg_random.hpp>

namespace terraformer
{
	using random_generator = pcg_engines::oneseq_dxsm_128_64;
	using rng_seed_type = __int128;

	struct simulation_description
	{
		rng_seed_type rng_seed;
		domain_size_description domain_size;
		initial_heightmap_description initial_heightmap;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, simulation_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "rng_seed",
			.display_name ="Random generator seed",
			.description = "Sets the initial value for the random bit generator",
			.widget = std::tuple{
				textbox{
					.value_converter = hash_string_converter<rng_seed_type>{},
					.binding = std::ref(params.get().rng_seed),
					.min_width = 32
				},
				input_button{
					.value_generator = [seed_src = random_bit_source{}]() {
						return seed_src.get<rng_seed_type>();
					},
					.binding = std::ref(params.get().rng_seed),
					.label = "🂠",
					.description = "Gererate a seed value",
				}
			}
		});

		form.insert(
			field{
				.name = "domain_size",
				.display_name = "Domain size",
				.description = "Sets the size of the domain",
				.widget = subform{
					.binding = std::ref(params.get().domain_size)
				}
			}
		);

		form.insert(
			field{
				.name = "initial_heightmap",
				.display_name = "Initial heightmap",
				.description = "Sets parameters for initial heightmap generation",
				.widget = subform{
					.binding = std::ref(params.get().initial_heightmap)
				}
			}
		);
	}
}

#endif
