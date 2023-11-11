//@	{"dependencies_extra":[{"ref":"./filtered_noise_generator.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTERED_NOISE_GENERATOR_HPP
#define TERRAFORMER_FILTERED_NOISE_GENERATOR_HPP

#include "./dimensions.hpp"
#include "./calculator.hpp"
#include "./polyline.hpp"

#include "lib/formbuilder/formfield.hpp"
#include "lib/filters/dft_engine.hpp"
//#include "lib/common/utils.hpp"
#include "lib/common/output_range.hpp"
#include "lib/interp.hpp"
#include "lib/boundary_sampling_policies.hpp"

#include <random>
#include <algorithm>
#include <memory>
#include <span>

namespace terraformer
{
	struct filtered_noise_description_1d
	{
		domain_length lambda_0;
		filter_order hp_order;
		filter_order lp_order;
	};

	template<class Form, class T>
	requires(std::is_same_v<std::remove_cvref_t<T>, filtered_noise_description_1d>)
	void bind(Form& form, std::reference_wrapper<T> params)
	{
		form.insert(field{
			.name = "lambda_0",
			.display_name = "Wavelength",
			.description = "Sets the dominant wavelength",
			.widget = numeric_input_log{
				.binding = std::ref(params.get().lambda_0),
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

	void apply_filter(std::span<float const> input, float* output, float lambda_max, filtered_noise_description_1d const& params);

	class filtered_noise_generator_1d
	{
	public:
		template<class Rng>
		explicit filtered_noise_generator_1d(Rng&& rng,
			size_t point_count,
			float dx,
			filtered_noise_description_1d const& params):
			m_signal_length{2*point_count},
			m_signal{std::make_unique<float[]>(m_signal_length)},
			m_dx{dx}
		{
			std::uniform_real_distribution U{0.0f, 1.0f};
			auto noise = std::make_unique_for_overwrite<float[]>(m_signal_length);
			for(size_t k = 0; k != m_signal_length; ++k)
			{ noise[k] = U(rng); }
			apply_filter(std::span{noise.get(), m_signal_length}, m_signal.get(), 2.0f*static_cast<float>(point_count)*dx, params);
		}

		float operator()(float x) const
		{ return interp(std::span{m_signal.get(), m_signal_length}, x/m_dx, wrap_around_at_boundary{}); }

		float dx() const { return m_dx; }

		size_t point_count() const { return m_signal_length/2; }

	private:
		size_t m_signal_length;
		std::unique_ptr<float[]> m_signal;
		float m_dx;
	};


	std::vector<location> generate(
		filtered_noise_generator_1d const& wave_xy,
		float amp_xy,
		float peak_loc_xy,
		filtered_noise_generator_1d const& wave_xz,
		float amp_xz,
		float peak_loc_xz,
		polyline_location_params const& line_params);

	template<class Rng>
	auto generate(Rng&& rng,
		filtered_noise_description_1d const& wave_xy,
		float amp_xy,
		float peak_loc_xy,
		filtered_noise_description_1d const& wave_xz,
		float amp_xz,
		float peak_loc_xz,
		polyline_location_params const& line_params)
	{
		return generate(
			filtered_noise_generator_1d{
				rng,
				line_params.point_count,
				line_params.dx,
				wave_xy
			},
			amp_xy,
			peak_loc_xy,
			filtered_noise_generator_1d{
				rng,
				line_params.point_count,
				line_params.dx,
				wave_xz
			},
			amp_xz,
			peak_loc_xz,
			line_params
		);
	}
}

#endif