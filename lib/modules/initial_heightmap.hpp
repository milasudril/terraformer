#ifndef TERRAFORMER_INITIAL_HEIGHTMAP_HPP
#define TERRAFORMER_INITIAL_HEIGHTMAP_HPP

namespace terraformer
{
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

	template<class Form>
	void bind(Form& form, corners& params)
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
				.binding = std::ref(params.ne.elevation)
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
				.binding = std::ref(params.ne.elevation)
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
				.binding = std::ref(params.sw.elevation)
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
				.binding = std::ref(params.se.elevation)
			},
		});
	}
	
	
	struct initial_heightmap
	{
		struct corners corners;
	};
	
	template<class Form>
	void bind(Form& form, initial_heightmap& params)
	{
		form.insert(
			field{
				.name = "corners",
				.display_name = "Corner elevations",
				.description = "Sets elevation at domain corners",
				.widget = subform{
					.binding = std::ref(params.corners)
				}
			}
		);
	}
}

#endif