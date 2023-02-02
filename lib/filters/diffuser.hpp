#ifndef TERRAFORMER_FILTERS_DIFFUSER_HPP
#define TERRAFORMER_FILTERS_DIFFUSER_HPP

#include "lib/execution/signaling_counter.hpp"
#include "lib/execution/notifying_task.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/common/double_buffer.hpp"

#include <type_traits>
#include <concepts>
#include <cassert>

namespace terraformer
{
	template<class T, class ConcentrationVector>
	concept diffusion_source_function = requires(T src, uint32_t x, uint32_t y)
	{
		{src(x, y)} -> std::same_as<ConcentrationVector>;
	};

	template<class ConcentrationVector>
	struct dirichlet_boundary_pixel
	{
		float weight;
		ConcentrationVector value;
	};

	template<class T, class ConcentrationVector>
	concept dirichlet_boundary_function = requires(T b, uint32_t x, uint32_t y)
	{
		{b(x, y)} -> std::same_as<dirichlet_boundary_pixel<ConcentrationVector>>;
	};

	template<class T, class ConcentrationVector>
	concept diffusion_coeff_vector = std::is_same_v<float, T> &&
	requires (T val, ConcentrationVector c)
	{
		{val*c} -> std::same_as<ConcentrationVector>;
	};

	inline constexpr std::array<std::array<float, 3>, 3> laplace_kernel{
		std::array<float, 3>{0.0f,  1.0f,  0.0f},
		std::array<float, 3>{1.0f, -4.0f,  1.0f},
		std::array<float, 3>{0.0f,  1.0f,  0.0f},
	};

	template<class DiffCoeff, class Boundary, class Src>
	struct diffusion_params
	{
		DiffCoeff D;  // Must be smaller than one
		Boundary boundary;
		Src source;
	};

	struct scanline_range
	{
		uint32_t begin;
		uint32_t end;
	};

	template<class ConcentrationVector,
		diffusion_coeff_vector<ConcentrationVector> DiffCoeff,
		dirichlet_boundary_function<ConcentrationVector> Boundary,
		diffusion_source_function<ConcentrationVector> Src>
	auto run_diffusion_step(span_2d<ConcentrationVector> output_buffer,
		span_2d<ConcentrationVector const> input_buffer,
		diffusion_params<DiffCoeff, Boundary, Src> const& params,
		scanline_range range)
	{
		assert(output_buffer.width() == input_buffer.width());
		assert(output_buffer.height() == input_buffer.height());
		assert(output_buffer.data() != input_buffer.data());

		auto const h = output_buffer.height();
		auto const w = output_buffer.width();

		float max_delta{};

		for(uint32_t y = range.begin; y != range.end; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{
				ConcentrationVector laplace{};
				for(uint32_t eta = 0; eta != 3; ++eta)
				{
					for(uint32_t xi = 0; xi != 3; ++xi)
					{
						static_assert(std::is_same_v<decltype(x - 1), uint32_t>);
						laplace += laplace_kernel[eta][xi]
							*input_buffer((x - 1 + xi + w)%w, (y - 1 + eta + h)%h);
					}
				}

				auto const bv = params.boundary(x, y);
				auto const old_val = input_buffer(x, y);
				auto const ds = old_val + (0.25f*params.D*laplace + params.source(x, y));
				auto const new_val = bv.value*bv.weight + (1.0f - bv.weight)*ds;
				output_buffer(x, y) = new_val;

				using geosimd::norm;
				max_delta = std::max(max_delta, norm(new_val - old_val));
			}
		}

		return max_delta;
	}

	template<class ConcentrationVector,
		diffusion_coeff_vector<ConcentrationVector> DiffCoeff,
		dirichlet_boundary_function<ConcentrationVector> Boundary,
		diffusion_source_function<ConcentrationVector> Src>
	auto run_diffusion_step(span_2d<ConcentrationVector> output_buffer,
		span_2d<ConcentrationVector const> input_buffer,
		diffusion_params<DiffCoeff, Boundary, Src> const& params)
	{
		return run_diffusion_step(output_buffer, input_buffer, params, scanline_range{
			.begin = 0,
			.end = output_buffer.height()
		});
	}

	template<class ConcentrationVector, class DiffCoeff, class Boundary, class Src>
	using diffusion_step = void (*)(float&,
		span_2d<ConcentrationVector>,
		span_2d<ConcentrationVector const>,
		diffusion_params<DiffCoeff, Boundary, Src> const&,
		scanline_range);

	template<class ConcentrationVector, class DiffCoeff, class Boundary, class Src>
	using diffusion_step_execution = notifying_task<
		std::reference_wrapper<signaling_counter>,
		diffusion_step<ConcentrationVector, DiffCoeff, Boundary, Src>,
		std::reference_wrapper<float>,
		span_2d<ConcentrationVector>,
		span_2d<ConcentrationVector const>,
		std::reference_wrapper<diffusion_params<DiffCoeff, Boundary, Src> const>,
		scanline_range
		>;

	template<class DiffusionStepExecutor, class ConcentrationVector, class DiffCoeff, class Boundary, class Src>
	concept diffusion_step_executor = requires(DiffusionStepExecutor e,
		diffusion_step_execution<ConcentrationVector, DiffCoeff, Boundary, Src> task)
	{
		{e.run(std::move(task))} -> std::same_as<void>;
	};

	template<template<class> class DiffusionStepExecutor,
		class ConcentrationVector,
		diffusion_coeff_vector<ConcentrationVector> DiffCoeff,
		dirichlet_boundary_function<ConcentrationVector> Boundary,
		diffusion_source_function<ConcentrationVector> Src>
	requires (diffusion_step_executor<
		DiffusionStepExecutor<
			diffusion_step_execution<
				ConcentrationVector,
				DiffCoeff,
				Boundary,
				Src>
			>,
		   ConcentrationVector,
		   DiffCoeff,
		   Boundary,
		   Src
		>)
	class diffusion_solver
	{
	public:
		using buffer_type = basic_image<ConcentrationVector>;

		explicit diffusion_solver(uint32_t num_workers,
			double_buffer<buffer_type>& buffers,
			diffusion_params<DiffCoeff, Boundary, Src>&& params):
			m_executor{num_workers},
			m_buffers{buffers},
			m_params{std::move(params)}
		{}

		auto operator()()
		{
			auto const n_workers = std::size(m_executor);
			auto retvals = std::make_unique_for_overwrite<float[]>(n_workers);

			auto const domain_height = m_buffers.get().back().height();
			auto const batch_size = 1 + (domain_height - 1)/static_cast<uint32_t>(n_workers);
			signaling_counter counter{n_workers};
			for(size_t k = 0; k != n_workers; ++k)
			{
				m_executor.run(notifying_task{
					std::ref(counter),
					+[](float& max_delta,
						span_2d<ConcentrationVector> output_buffer,
						span_2d<ConcentrationVector const> input_buffer,
						diffusion_params<DiffCoeff, Boundary, Src> const& params,
						scanline_range range)
					{
						max_delta = run_diffusion_step(output_buffer, input_buffer, params, range);
					},
					std::ref(retvals[k]),
					m_buffers.get().back().pixels(),
					m_buffers.get().front().pixels(),
					std::cref(m_params),
					scanline_range{
						.begin = static_cast<uint32_t>(k*batch_size),
						.end = std::min(domain_height, static_cast<uint32_t>((k + 1)*batch_size))
					}
				});
			}
			counter.wait();
			m_buffers.get().swap();
			return *std::max_element(retvals.get(), retvals.get() + n_workers);
		}

	private:
		DiffusionStepExecutor<diffusion_step_execution<ConcentrationVector, DiffCoeff, Boundary, Src>> m_executor;
		std::reference_wrapper<double_buffer<buffer_type>> m_buffers;
		diffusion_params<DiffCoeff, Boundary, Src> m_params;
	};


	template<template<class> class DiffusionStepExecutor,
		class ConcentrationVector,
		diffusion_coeff_vector<ConcentrationVector> DiffCoeff,
		dirichlet_boundary_function<ConcentrationVector> Boundary,
		diffusion_source_function<ConcentrationVector> Src>
	requires (diffusion_step_executor<
		DiffusionStepExecutor<
			diffusion_step_execution<
				ConcentrationVector,
				DiffCoeff,
				Boundary,
				Src>
			>,
		   ConcentrationVector,
		   DiffCoeff,
		   Boundary,
		   Src
		>)
	auto make_diffusion_solver(uint32_t num_workers,
		double_buffer<basic_image<ConcentrationVector>>& buffers,
		diffusion_params<DiffCoeff, Boundary, Src>&& params)
	{
		return diffusion_solver<DiffusionStepExecutor, ConcentrationVector, DiffCoeff, Boundary, Src>
			{num_workers, buffers, std::move(params)};
	}


	template<template<class> class DiffusionStepExecutor,
		class ConcentrationVector,
		diffusion_coeff_vector<ConcentrationVector> DiffCoeff,
		dirichlet_boundary_function<ConcentrationVector> Boundary,
		diffusion_source_function<ConcentrationVector> Src>
	requires (diffusion_step_executor<
		DiffusionStepExecutor<
			diffusion_step_execution<
				ConcentrationVector,
				DiffCoeff,
				Boundary,
				Src>
			>,
		   ConcentrationVector,
		   DiffCoeff,
		   Boundary,
		   Src
		>)
	auto solve_laplace(uint32_t num_workers,
		double_buffer<basic_image<ConcentrationVector>>& buffers,
		diffusion_params<DiffCoeff, Boundary, Src>&& params,
		float tolerance)
	{
		auto diffuser = make_diffusion_solver<DiffusionStepExecutor>(
			num_workers, buffers, std::move(params));
		while(true)
		{
			auto const delta = diffuser();

			if(delta < tolerance)
			{ return delta; }
		}
	}
}

#endif