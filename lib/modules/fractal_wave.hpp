//@	{"dependencies_extra":[{"ref": "./fractal_wave.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FRACTALWAVE_HPP
#define TERRAFORMER_FRACTALWAVE_HPP

#include "./polyline.hpp"
#include "./dimensions.hpp"
#include "./calculator.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/utils.hpp"
#include "lib/common/output_range.hpp"
#include "lib/formbuilder/formfield.hpp"
#include "lib/pixel_store/image.hpp"

#include <numbers>
#include <random>
#include <cmath>
#include <array>

namespace terraformer
{
	struct wave_size_progression
	{
		scaling_factor factor;
		noise_amplitude scaling_noise;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, wave_size_progression>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "factor",
			.display_name = "Scaling factor",
			.description = "Controls the relative size between two consecutive elements",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().factor),
				.value_converter = calculator{}
			}
		});

		form.insert(field{
			.name = "scaling_noise",
			.display_name = "Scaling noise",
			.description = "Controls the amount of noise to add to the scaling factor",
			.widget = numeric_input{
				.binding = std::ref(params.get().scaling_noise),
				.value_converter = calculator{}
			}
		});
	}

	template<class Rng>
	inline float get_value(wave_size_progression const& val, size_t k, Rng&& rng)
	{
		std::uniform_real_distribution U{-0.5f, 0.5f};
		return std::pow(val.factor, -(static_cast<float>(k) + val.scaling_noise*U(rng)));
	}

	struct wave_phase_progression
	{
		phase_offset offset;
		noise_amplitude offset_noise;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, wave_phase_progression>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "offset",
			.display_name = "Offset",
			.description = "Controls the offset between two consecutive elements",
			.widget = numeric_input{
				.binding = std::ref(params.get().offset),
				.value_converter = calculator{}
			}
		});

		form.insert(field{
			.name = "offset_noise",
			.display_name = "Offset noise",
			.description = "Controls the amount of noise to add to the offset",
			.widget = numeric_input{
				.binding = std::ref(params.get().offset_noise),
				.value_converter = calculator{}
			}
		});
	}

	template<class Rng>
	inline float get_value(wave_phase_progression const& val, size_t k, Rng&& rng)
	{
		std::uniform_real_distribution U{-0.5f, 0.5f};
		return -(static_cast<float>(k)*val.offset + val.offset_noise*U(rng));
	}

	struct wave_params
	{
		domain_length wavelength;
		global_phase phase;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, wave_params>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "wavelength",
			.display_name = "Wavelength",
			.description = "Controls the wavelength",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().wavelength),
				.value_converter = calculator{}
			}
		});

		form.insert(field{
			.name = "phase",
			.display_name = "Phase",
			.description = "Controls the phase",
			.widget = numeric_input{
				.binding = std::ref(params.get().phase),
				.value_converter = calculator{}
			}
		});
	}

	class fractal_wave
	{
	public:
		struct params
		{
			wave_size_progression amplitude;
			wave_size_progression wavelength;
			wave_phase_progression phase;
		};

		static auto compute_number_of_waves(params const& params)
		{
			auto const k_amp = 14.0f/std::log2(params.amplitude.factor);
			auto const k_lambda = 14.0f/std::log2(params.wavelength.factor);
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
		struct wave_component
		{
			float amplitude;
			float wavelength;
			float phase;
		};

		float generate_unnormalized(float x) const
		{
			auto sum = 0.0f;
			auto constexpr twopi = 2.0f*std::numbers::pi_v<float>;
			for(size_t k = m_num_waves; k != 0; --k)
			{
				auto const amplitude = m_components[k - 1].amplitude;
				auto const wavelength = m_components[k - 1].wavelength;
				auto const phase = m_components[k - 1].phase;

				sum += amplitude*approx_sine(twopi*(x/wavelength + phase + 0.25f));
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


	class bump_field_generator
	{
	public:
		struct params
		{
			fractal_wave::params x;
			fractal_wave::params y;
		};

		static auto compute_number_of_waves(fractal_wave::params const& params)
		{
			auto const k_amp = 14.0f/std::log2(params.amplitude.factor);
			auto const k_lambda = 14.0f/std::log2(params.wavelength.factor);
			auto const k_max = 8.0f;

			return round_to_odd(std::max(std::min(k_max, std::min(k_amp, k_lambda)), 3.0f));
		}

		static auto compute_number_of_waves(params const& params)
		{
			auto size_x = compute_number_of_waves(params.x);
			auto size_y = compute_number_of_waves(params.y);

			return std::max(size_x, size_y);
		}

		using vec4f_t = geosimd::vector_storage<float, 4>;

		static auto pow(vec4f_t x, vec4f_t y)
		{
			for(int k = 0; k != 4; ++k)
			{ x[k] = std::pow(x[k], y[k]); }
			return x;
		}

		template<class Rng>
		explicit bump_field_generator(Rng&& rng, params const& params):
			m_size{compute_number_of_waves(params)},
			m_components{std::make_unique_for_overwrite<wave_component[]>(m_size*m_size)}
		{
			auto const size = static_cast<int32_t>(m_size);
			size_t index = 0;
			auto const n = m_size*m_size;

			scaling const amp_scale{
				std::log2(params.x.amplitude.factor),
				std::log2(params.y.amplitude.factor),
				0.0f
			};

			displacement const amp_scale_noise{
				static_cast<float>(params.x.amplitude.scaling_noise),
				static_cast<float>(params.y.amplitude.scaling_noise),
				0.0f
			};

			vec4f_t const wavelength_scale{
				params.x.wavelength.factor,
				params.y.wavelength.factor,
				0.0f,
				0.0f
			};

			vec4f_t const wavelength_scale_noise{
				static_cast<float>(params.x.wavelength.scaling_noise),
				static_cast<float>(params.y.wavelength.scaling_noise),
				0.0f,
				0.0f
			};

			scaling const phase_offset{
				static_cast<float>(params.x.phase.offset),
				static_cast<float>(params.y.phase.offset),
				0.0f
			};

			displacement const phase_offset_noise{
				static_cast<float>(params.x.phase.offset_noise),
				static_cast<float>(params.y.phase.offset_noise),
				0.0f
			};

			std::uniform_real_distribution U{-0.5f, 0.5f};

			for(int32_t k = 0; k != size; ++k)
			{
				for(int32_t l = 0; l != size; ++l)
				{
					displacement const r{
						static_cast<float>(l),
						static_cast<float>(k),
						0.0f
					};

					displacement const wave_vector{
						pow(wavelength_scale,
							vec4f_t{
								static_cast<float>(l),
								static_cast<float>(k),
								1.0f,
								1.0f
							}
							+ wavelength_scale_noise*vec4f_t{U(rng), U(rng), 0.0f, 0.0f}
						)
					};

					m_components[index] = wave_component{
						.amplitude = std::exp2(
							-norm(
								(r + amp_scale_noise.apply(scaling{U(rng), U(rng), 0.0f})).apply(amp_scale)
							)
						),
						.wave_vector = wave_vector,
						.phase = norm(r.apply(phase_offset) + phase_offset_noise.apply(scaling{U(rng), U(rng), 0.0f}))
					};

					++index;
				}
			}

			std::sort(m_components.get(), m_components.get() + n, [](auto const& a, auto const& b){
				return a.amplitude < b.amplitude;
			});
		}

		float operator()(float x, float y) const
		{
			constexpr auto twopi = 2.0f*std::numbers::pi_v<float>;
			auto sum = 0.0f;
			auto const n = m_size*m_size - 1;
			displacement const vec{x, y, 0.0f};
			for(size_t k = 0; k != n; ++k)
			{
				auto const& component = m_components[k];
				auto const q1 = component.wave_vector;
				auto const q2 = component.wave_vector.apply(scaling{-1.0f, 1.0f, 1.0f});
				auto const q3 = component.wave_vector.apply(scaling{-1.0f, -1.0f ,1.0f});
				auto const q4 = component.wave_vector.apply(scaling{1.0f, -1.0f, 1.0f});
				sum += component.amplitude*(
					 approx_sine(twopi*(inner_product(vec, q1) + component.phase + 0.25f))
					+approx_sine(twopi*(inner_product(vec, q2) + component.phase + 0.25f))
					+approx_sine(twopi*(inner_product(vec, q3) + component.phase + 0.25f))
					+approx_sine(twopi*(inner_product(vec, q4) + component.phase + 0.25f))
				);
			}
			return sum;
		}

	private:

		struct wave_component
		{
			float amplitude;
			displacement wave_vector;
			float phase;
		};

		size_t m_size;
		std::unique_ptr<wave_component[]> m_components;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, bump_field_generator::params>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "x",
			.display_name = "x",
			.description = "Controls the shape of the x wave",
			.widget = subform{
				.binding = std::ref(params.get().x)
			}
		});

		form.insert(field{
			.name = "y",
			.display_name = "y",
			.description = "Controls the shape of the y wave",
			.widget = subform{
				.binding = std::ref(params.get().y)
			}
		});
	}


	struct bump_field_description
	{
		bump_field_generator::params shape;
		wave_params wave_properties_x;
		wave_params wave_properties_y;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, bump_field_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "wave_properties_x",
			.display_name = "Wave properties x",
			.description = "Controls the total wave in x direction",
			.widget = subform{
				.binding = std::ref(params.get().wave_properties_x)
			}
		});

		form.insert(field{
			.name = "wave_properties_y",
			.display_name = "Wave properties y",
			.description = "Controls the total wave in y direction",
			.widget = subform{
				.binding = std::ref(params.get().wave_properties_y)
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
}

#endif