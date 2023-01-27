#ifndef TERRAFORMER_FILTERS_DIFFUSER_HPP
#define TERRAFORMER_FILTERS_DIFFUSER_HPP

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
	concept diffusion_coeff_vector = requires(T val, ConcentrationVector c)
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
		float dt;  // Must be smaller than 4/(2*D)
		DiffCoeff D;
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

		using geosimd::norm;
		using abs_value = decltype(norm(std::declval<ConcentrationVector>()));

		abs_value ret{};

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
				auto const ds = old_val + params.dt*(0.25f*params.D*laplace + params.source(x, y));
				auto const new_val = bv.value*bv.weight + (1.0f - bv.weight)*ds;
				output_buffer(x, y) = new_val;

				ret = std::max(ret, norm(new_val - old_val));
			}
		}

		return ret;
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
}

#endif