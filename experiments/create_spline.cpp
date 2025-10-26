#include "lib/math_utils/cubic_spline.hpp"

#include <pretty/plot.hpp>

namespace terraformer
{
	single_array<polynomial<displacement, 3>> create_spline(span<location const> curve);
}

terraformer::single_array<terraformer::polynomial<terraformer::displacement, 3>>
terraformer::create_spline(span<location const> curve)
{
	if(std::size(curve).get() < 2)
	{ return single_array<polynomial<displacement, 3>>{}; }

	single_array ret{array_size<polynomial<displacement, 3>>{std::size(curve).get() - 1}};

	auto const indices = curve.element_indices(1);
	cubic_spline_control_point prev_control_point{
		.y = curve.front(),
		.ddx = curve[indices.front()] - curve.front()
	};

	for(auto k = indices.front(); k != indices.back(); ++k)
	{
		cubic_spline_control_point const control_point{
			.y = curve[k],
			.ddx = 0.5f*(curve[k + 1] - curve[k - 1])
		};

		ret[decltype(ret)::index_type{(k - 1).get()}] = make_polynomial(prev_control_point, control_point);
		prev_control_point = control_point;
	}

	ret.back() = make_polynomial(
		prev_control_point,
		cubic_spline_control_point{
			.y = curve.back(),
			.ddx = curve.back() - curve[indices.back() - 1]
		}
	);

	return ret;
}

int main()
{
	constexpr std::array locs{
		terraformer::location{0.0f, 0.0f, 0.0f},
		terraformer::location{1.0f, 1.0f, 0.0f},
		terraformer::location{2.0f, 0.0f, 0.0f}
	};

	auto polynomials = create_spline(
		terraformer::span{
			std::begin(locs),
			std::end(locs)
		}
	);

	std::vector<terraformer::location> interp;
	for(auto const& p : polynomials)
	{
		for(size_t k = 0; k != 16; ++k)
		{
			auto const t = (static_cast<float>(k) + 0.5f)/16.0f;
			interp.push_back(terraformer::location{} + p(t));
		}
	}
	pretty::plot(interp);
	pretty::print(interp);
}