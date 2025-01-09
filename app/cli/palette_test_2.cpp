#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"

#include <pretty/base.hpp>
#include <pretty/plot.hpp>
#include <array>

int main()
{
	std::array<float, 4> x_vals{0.0f, 0.5f, 0.75f, 1.0f};
	std::array<float, 4> y_vals{0.f, 1.0f/3.0f, 2.0f/3.0f, 1.0f};
	pretty::print(x_vals);
	pretty::print(y_vals);
	std::vector<std::pair<float, float>> interp_vals;
	for(size_t k = 0; k != 17; ++k)
	{
		auto const x = static_cast<float>(k)/static_cast<float>(16);
		auto const y = terraformer::interp(
			x_vals,
			y_vals,
			x,
			terraformer::clamp_at_boundary{}
		);
		interp_vals.push_back(std::pair{x, y});
	}
	pretty::print(interp_vals);
	pretty::plot(interp_vals);
}