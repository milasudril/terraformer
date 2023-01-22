#ifndef TERRAFORMER_FILTERS_DIFFUSER_HPP
#define TERRAFORMER_FILTERS_DIFFUSER_HPP

#include <type_traits>
#include <concepts>

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

	template<class ConcentrationVector,
		diffusion_coeff_vector<ConcentrationVector> DiffCoeff,
		dirichlet_boundary_function<ConcentrationVector> BoundaryFunuc,
		diffusion_source_function<ConcentrationVector> Src>
	struct diffusion_params
	{
		float dt;
		DiffCoeff D;
		BoundaryFunc boundary;
		SourceFunc source;
	};

	inline constexpr std::array<std::array<float, 3>, 3> laplace_kernel{
		std::array<float>{0.0f,  1.0f,  0.0f},
		std::array<float>{1.0f, -2.0f,  1.0f},
		std::array<float>{0.0f,  1.0f,  0.0f},
	};

	template<class ConcentrationVector, class DiffCoeff, class BoundaryFunc, class SourceFunc>
	void run_diffusion_step(span_2d<ConcentrationVector> output_buffer,
		span_2d<ConcentrationVector const> input_buffer,
		diffusion_params<ConcentrationVector, DiffCoeff, BoundaryFunc, SourceFunc> const& params)
	{
		assert(output_buffer.width() == input_buffer.width());
		assert(output_buffer.height() == input_buffer.height());

		auto const h = output_buffer.height();
		auto const w = output_buffer.width();

		for(uint32_t y = 0; y != h; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{
				PixleType laplace{};
				for(uint32_t eta = 0; eta != 3; ++eta)
				{
					for(uint32_t xi = 0; xi != 3; ++xi)
					{
						laplace += laplace_kernel[eta][xi]
							*input_buffer((x - 1 + xi + w)%w, (y - 1 + eta + h)%h);
					}
				}

				auto const bv = params.boundary(x, y);
				auto const diff_step = input_buffer(x, y) + params.dt*(0.25f*params.D*laplace + src(x, y));
				output_buffer(x, y) = bv.value*bv.weight + (1.0f - bv.weight)*diff_step;
			}
		}
	}
}

#endif