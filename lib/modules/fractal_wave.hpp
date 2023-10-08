//@	{"dependencies_extra":[{"ref": "./fractal_wave.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FRACTALWAVE_HPP
#define TERRAFORMER_FRACTALWAVE_HPP

#include "./polyline.hpp"
#include "lib/common/utils.hpp"
#include "lib/common/output_range.hpp"
#include "lib/formbuilder/string_converter.hpp"
#include "lib/formbuilder/formfield.hpp"

#include <numbers>
#include <random>
#include <cmath>
#include <array>

namespace terraformer
{
	struct exponential_progression
	{
		float scaling_factor;
		float scaling_noise;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, exponential_progression>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "scaling_factor",
			.display_name = "Scaling factor",
			.description = "Controls the relative size between two consecutive elements",
			.widget = std::tuple{
				knob{
					.min = 0.0f,
					.max = 2.0f,
					.binding = std::ref(params.get().scaling_factor),
					.mapping = numeric_input_mapping_type::log
				},
				textbox{
					.value_converter = num_string_converter{
						.range = closed_open_interval{
							.min = 1.0f,
							.max = std::numeric_limits<float>::infinity()
						}
					},
					.binding = std::ref(params.get().scaling_factor)
				}
			}
		});

		form.insert(field{
			.name = "scaling_noise",
			.display_name = "Scaling noise",
			.description = "Controls the amount of noise to add to the scaling factor",
			.widget = std::tuple{
				knob{
					.min = 0.0f,
					.max = 2.0f,
					.binding = std::ref(params.get().scaling_noise)
				},
				textbox{
					.value_converter = num_string_converter{
						.range = closed_open_interval{
							.min = 0.0f,
							.max = std::numeric_limits<float>::infinity()
						}
					},
					.binding = std::ref(params.get().scaling_noise)
				}
			}
		});
	}

	template<class Rng>
	inline float get_value(exponential_progression const& val, size_t k, Rng&& rng)
	{
		std::uniform_real_distribution U{-0.5f, 0.5f};
		return std::pow(val.scaling_factor, -(static_cast<float>(k) + val.scaling_noise*U(rng)));
	}

	struct linear_mod_progression
	{
		float offset;
		float offset_noise;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, linear_mod_progression>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "offset",
			.display_name = "Offset",
			.description = "Controls the offset between two consecutive elements",
			.widget = std::tuple{
				knob{
					.min = -0.5f,
					.max = 0.5f,
					.binding = std::ref(params.get().offset)
				},
				textbox{
					.value_converter = num_string_converter{
						.range = closed_closed_interval{
							.min = -0.5f,
							.max = 0.5f
						}
					},
					.binding = std::ref(params.get().offset)
				}
			}
		});

		form.insert(field{
			.name = "offset_noise",
			.display_name = "Offset noise",
			.description = "Controls the amount of noise to add to the offset",
			.widget = std::tuple{
				knob{
					.min = 0.0f,
					.max = 1.f,
					.binding = std::ref(params.get().offset_noise)
				},
				textbox{
					.value_converter = num_string_converter{
						.range = closed_closed_interval{
							.min = 0.0f,
							.max = 1.0f
						}
					},
					.binding = std::ref(params.get().offset_noise)
				}
			}
		});
	}

	template<class Rng>
	inline float get_value(linear_mod_progression const& val, size_t k, Rng&& rng)
	{
		std::uniform_real_distribution U{-0.5f, 0.5f};
		return -(static_cast<float>(k)*val.offset + val.offset_noise*U(rng));
	}

	struct wave_params
	{
		float wavelength;
		float phase;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, wave_params>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "wavelength",
			.display_name = "Wavelength",
			.description = "Controls the wavelength",
			.widget = std::tuple{
				knob{
					.min = 8.0f,
					.max = 17.0f,
					.binding = std::ref(params.get().wavelength),
					.mapping = numeric_input_mapping_type::log
				},
				textbox{
					.value_converter = num_string_converter{
						.range = open_open_interval{
							.min = 0.0f,
							.max = std::numeric_limits<float>::infinity()
						}
					},
					.binding = std::ref(params.get().wavelength)
				}
			}
		});

		form.insert(field{
			.name = "phase",
			.display_name = "Phase",
			.description = "Controls the phase",
			.widget = std::tuple{
				knob{
					.min = 0.0f,
					.max = 8.0f,
					.binding = std::ref(params.get().phase)
				},
				textbox{
					.value_converter = num_string_converter{
						.range = closed_closed_interval{
							.min = 0.0f,
							.max = 8.0f
						}
					},
					.binding = std::ref(params.get().phase)
				}
			}
		});
	}

	struct wave_component
	{
		float amplitude;
		float wavelength;
		float phase;
	};

	class fractal_wave
	{
	public:
		struct params
		{
			exponential_progression amplitude;
			exponential_progression wavelength;
			linear_mod_progression phase;
		};

		static auto compute_number_of_waves(params const& params)
		{
			auto const k_amp = 14.0f/std::log2(params.amplitude.scaling_factor);
			auto const k_lambda = 14.0f/std::log2(params.wavelength.scaling_factor);
			auto const k_max = 64.0f;
			return static_cast<size_t>(std::min(k_max, std::min(k_amp, k_lambda)) + 0.5f) + 1;
		}

		template<class Rng>
		explicit fractal_wave(Rng&& rng, params const& params):
		m_num_waves{compute_number_of_waves(params)},
		m_components{std::make_unique_for_overwrite<wave_component[]>(m_num_waves)}
		{
			std::uniform_real_distribution U{-0.5f, 0.5f};
			for(size_t k = 0; k != m_num_waves; ++k)
			{
				m_components[k] = wave_component{
					.amplitude = get_value(params.amplitude, k, rng),
					.wavelength = get_value(params.wavelength, k, rng),
					.phase = get_value(params.phase, k, rng)
				};
			}

			auto min = std::numeric_limits<float>::infinity();
			auto max = -std::numeric_limits<float>::infinity();
			for(size_t k = 0; k != 1024; ++k)
			{
				auto x = static_cast<float>(k)/32.0f;
				auto val = generate_unnormalized(x);
				min = std::min(val, min);
				max = std::max(val, max);
			}
			m_offset = 0.5f*(max + min);
			m_amplitude = 0.5f*(max - min);
		}

		auto operator()(float x) const
		{ return (generate_unnormalized(x) - m_offset)/m_amplitude; }

	private:
		float generate_unnormalized(float x) const
		{
			auto sum = 0.0f;
			auto constexpr twopi = 2.0f*std::numbers::pi_v<float>;
			for(size_t k = m_num_waves; k != 0; --k)
			{
				auto const amplitude = m_components[k - 1].amplitude;
				auto const wavelength = m_components[k - 1].wavelength;
				auto const phase = m_components[k - 1].phase;

				sum += amplitude*approx_sine(twopi*(x/wavelength - phase + 0.25f));
			}
			return sum;
		}

		size_t m_num_waves;
		std::unique_ptr<wave_component[]> m_components;
		float m_amplitude;
		float m_offset;
	};

	struct fractal_wave_description
	{
		fractal_wave::params shape;
		wave_params wave_properties;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, fractal_wave::params>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "amplitude",
			.display_name = "Amplitude",
			.description = "Controls the amplitude progression",
			.widget = subform{
				.binding = std::ref(params.get().amplitude)
			}
		});

		form.insert(field{
			.name = "wavelength",
			.display_name = "Wavelength",
			.description = "Controls the wavelength progression",
			.widget = subform{
				.binding = std::ref(params.get().wavelength)
			}
		});

		form.insert(field{
			.name = "phase",
			.display_name = "Phase",
			.description = "Controls the phase progression",
			.widget = subform{
				.binding = std::ref(params.get().phase)
			}
		});
	}

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, fractal_wave_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "wave_properties",
			.display_name = "Wave properties",
			.description = "Controls the total wave",
			.widget = subform{
				.binding = std::ref(params.get().wave_properties)
			}
		});

		form.insert(field{
			.name = "shape",
			.display_name = "Shape",
			.description = "Controls the progression of individual wave components",
			.widget = subform{
				.binding = std::ref(params.get().shape)
			}
		});
	}

	std::vector<displacement> generate(fractal_wave const& wave,
		wave_params const& wave_params,
		output_range output_range,
		polyline_displacement_params const& line_params);

	std::vector<location> generate(fractal_wave const& wave,
		wave_params const& wave_params,
		output_range output_range,
		polyline_location_params const& line_params);

	template<class Rng, class ... Params>
	auto generate(Rng&& rng,
		fractal_wave_description const& wave_params,
		output_range output_range,
		Params&&... params)
	{
		return generate(fractal_wave{rng, wave_params.shape},
			wave_params.wave_properties,
			output_range,
			std::forward<Params>(params)...);
	}

	std::vector<location> generate(fractal_wave const& wave_xy,
		wave_params const& wave_xy_params,
		output_range xy_output_range,
		fractal_wave const& wave_xz,
		wave_params const& wave_xz_params,
		output_range xz_output_range,
		polyline_location_params const& line_params);

	template<class Rng, class ... Params>
	auto generate(Rng&& rng,
		fractal_wave_description const& wave_params_xy,
		output_range xy_output_range,
		fractal_wave_description const& wave_params_xz,
		output_range xz_output_range,
		Params&&... params)
	{
		return generate(fractal_wave{rng, wave_params_xy.shape},
			wave_params_xy.wave_properties,
			xy_output_range,
			fractal_wave{rng, wave_params_xz.shape},
			wave_params_xz.wave_properties,
			xz_output_range,
			std::forward<Params>(params)...);
	}
}

#endif