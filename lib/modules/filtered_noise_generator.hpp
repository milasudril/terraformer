//@	{"dependencies_extra":[{"ref":"./filtered_noise_generator.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTERED_NOISE_GENERATOR_HPP
#define TERRAFORMER_FILTERED_NOISE_GENERATOR_HPP

#include "./dimensions.hpp"
#include "./calculator.hpp"
#include "./polyline.hpp"

#include "lib/formbuilder/formfield.hpp"
#include "lib/math_utils/dft_engine.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/common/output_range.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"

#include <random>
#include <algorithm>
#include <memory>
#include <span>

namespace terraformer
{
	struct filtered_noise_1d_description
	{
		domain_length wavelength;
		filter_order hp_order;
		filter_order lp_order;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, filtered_noise_1d_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "wavelength",
			.display_name = "Wavelength",
			.description = "Sets the dominant wavelength",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().wavelength),
				.value_converter = calculator{}
			}
		});

		form.insert(field{
			.name = "hp_order",
			.display_name = "High-pass filter order",
			.description = "Sets the order of high-pass filter",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().hp_order),
				.value_converter = calculator{}
			}
		});

		form.insert(field{
			.name = "lp_order",
			.display_name = "Low-pass filter order",
			.description = "Sets the order of low-pass filter",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().lp_order),
				.value_converter = calculator{}
			}
		});
	}

	void apply_filter(
		std::span<float const> input,
		std::span<std::complex<double>> output,
		double lambda_max,
		filtered_noise_1d_description const& params);

	void apply_filter(
		std::span<float const> input,
		float* output,
		double lambda_max,
		filtered_noise_1d_description const& params);

	class filtered_noise_1d_generator
	{
	public:
		template<class Rng>
		explicit filtered_noise_1d_generator(Rng&& rng,
			size_t point_count,
			float dx,
			filtered_noise_1d_description const& params):
			m_signal_length{2*point_count},
			m_signal{std::make_unique<float[]>(m_signal_length)},
			m_dx{dx}
		{
			std::uniform_real_distribution U{0.0f, 1.0f};
			auto noise = std::make_unique_for_overwrite<float[]>(m_signal_length);
			for(size_t k = 0; k != m_signal_length; ++k)
			{ noise[k] = U(rng); }
			apply_filter(std::span{noise.get(), m_signal_length}, m_signal.get(), 2.0*static_cast<double>(point_count)*static_cast<double>(dx), params);
		}

		filtered_noise_1d_generator(filtered_noise_1d_generator&&) = default;

		filtered_noise_1d_generator& operator=(filtered_noise_1d_generator&&) = default;

		filtered_noise_1d_generator& operator=(filtered_noise_1d_generator const&) = delete;

		filtered_noise_1d_generator(filtered_noise_1d_generator const& src):
			m_signal_length{src.m_signal_length},
			m_signal{std::make_unique_for_overwrite<float[]>(m_signal_length)},
			m_dx{src.m_dx}
		{ std::copy_n(src.m_signal.get(), m_signal_length, m_signal.get()); }

		float operator()(float x) const
		{ return interp_eqdist(std::span{m_signal.get(), m_signal_length}, x/m_dx, wrap_around_at_boundary{}); }

		float dx() const { return m_dx; }

		size_t point_count() const { return m_signal_length/2; }

	private:
		size_t m_signal_length;
		std::unique_ptr<float[]> m_signal;
		float m_dx;
	};

	struct filtered_noise_1d_render_params
	{
		float amplitude;
		float peak_location;
		bool flip_direction;
		bool invert_displacement;
	};

	std::vector<location> generate(filtered_noise_1d_generator const& wave_xy,
		filtered_noise_1d_render_params const& render_params_xy,
		filtered_noise_1d_generator const& wave_xz,
		filtered_noise_1d_render_params const& render_params_xz,
		polyline_location_params const& line_params);

	template<class Rng>
	auto generate(Rng&& rng,
		filtered_noise_1d_description const& wave_xy,
		filtered_noise_1d_render_params const& render_params_xy,
		filtered_noise_1d_description const& wave_xz,
		filtered_noise_1d_render_params const& render_params_xz,
		polyline_location_params const& line_params)
	{
		return generate(
			filtered_noise_1d_generator{
				rng,
				line_params.point_count,
				line_params.dx,
				wave_xy
			},
			render_params_xy,
			filtered_noise_1d_generator{
				rng,
				line_params.point_count,
				line_params.dx,
				wave_xz
			},
			render_params_xz,
			line_params
		);
	}

	struct filtered_noise_2d_description
	{
		domain_length wavelength_x;
		domain_length wavelength_y;
		filter_order hp_order;
		filter_order lp_order;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, filtered_noise_2d_description>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "wavelength_x",
			.display_name = "Wavelength X",
			.description = "Sets the dominant wavelength in the x direction",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().wavelength_x),
				.value_converter = calculator{}
			}
		});

		form.insert(field{
			.name = "wavelength_y",
			.display_name = "Wavelength Y",
			.description = "Sets the dominant wavelength in the y direction",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().wavelength_y),
				.value_converter = calculator{}
			}
		});

		form.insert(field{
			.name = "hp_order",
			.display_name = "High-pass filter order",
			.description = "Sets the order of high-pass filter",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().hp_order),
				.value_converter = calculator{}
			}
		});

		form.insert(field{
			.name = "lp_order",
			.display_name = "Low-pass filter order",
			.description = "Sets the order of low-pass filter",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().lp_order),
				.value_converter = calculator{}
			}
		});
	}

	void apply_filter(
		span_2d<float const> input,
		span_2d<std::complex<double>> output,
		double lambda_max,
		filtered_noise_2d_description const& params);

	void apply_filter(span_2d<float const> input, span_2d<float> output, double lambda_max, filtered_noise_2d_description const& params);

	class filtered_noise_2d_generator
	{
	public:
		template<class Rng>
		explicit filtered_noise_2d_generator(
			Rng&& rng,
			span_2d_extents size,
			float dx,
			filtered_noise_2d_description const& params):
			m_signal{2*size.width, 2*size.height},
			m_dx{dx}
		{
			grayscale_image noise{m_signal.width(), m_signal.height()};
			generate(
				noise.pixels(),
				[&rng, U = std::uniform_real_distribution{0.0f, 1.0f}](auto...) mutable {
					return U(rng);
				}
			);

			apply_filter(
				noise.pixels(),
				m_signal.pixels(),
				2.0*static_cast<double>(std::min(size.width, size.height))*static_cast<double>(dx),
				params);
		}

		float operator()(float x, float y) const
		{ return interp(m_signal.pixels(), x/m_dx, y/m_dx, wrap_around_at_boundary{}); }

		float dx() const { return m_dx; }

	private:
		grayscale_image m_signal;
		float m_dx;
	};
}

#endif