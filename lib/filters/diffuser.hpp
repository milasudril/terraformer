#ifndef TERRAFORMER_DIFFUSER_HPP
#define TERRAFORMER_DIFFUSER_HPP

namespace terraformer
{
	template<class PixelType, class SourceFunction, class DirichletBoundary>
	void run_diffusion_step(span_2d<PixelType> output_buffer,
		span_2d<PixelType const> input_buffer,
		SourceFunction&& src = [](uint32_t, uint32_t){return PixelType{};},
		DirichletBoundary&& boundary = [](uint32_t, uint32_t){return std::optional<PixelType>{};})
	{
		assert(output_buffer.width() == input_buffer.width());
		assert(output_buffer.height() == input_buffer.height());
		assert(output_buffer.data() != input_buffer.data());

		auto const h = output_buffer.height();
		auto const w = output_buffer.width();

		constexpr std::array<std::array<float, 3>, 3> weights{
			std::array<float>{0.0f,  1.0f,  0.0f},
			std::array<float>{1.0f, -2.0f,  1.0f},
			std::array<float>{0.0f,  1.0f,  0.0f},
		}

		for(uint32_t y = 0; y != h; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{
				PixleType laplace{};

				for(uint32_t eta = 0; eta != 3; ++eta)
				{
					for(uint32_t xi = 0; xi != 3; ++xi)
					{
						laplace += weights[eta][xi]*input_buffer((x - 1 + xi + w)%w, (y - 1 + eta + h)%h);
					}
				}

				if(auto bv = boundary(x, y); bv.has_value())
				{ output_buffer(x, y) = *bv; }
				else
				{ output_buffer(x, y) = input_buffer(x, y) + td*(0.25f*laplace + src(x, y)); }
			}
		}
	}
};

#endif