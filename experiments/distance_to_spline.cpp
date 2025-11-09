#include "lib/math_utils/cubic_spline.hpp"

#include <pretty/plot.hpp>

int main()
{
	// Create test polynomial
	constexpr terraformer::cubic_spline_control_point first{
		.y = terraformer::location{},
		.ddx = 2.0f*terraformer::displacement{1.0f, 0.5f, 0.0f}
	};

	constexpr terraformer::cubic_spline_control_point second{
		.y = terraformer::location{1.0f, 1.0f, 0.0f},
		.ddx = terraformer::displacement{2.0f, 0.0f, 0.0f}
	};
	constexpr auto p = make_polynomial(first, second);

	// Plot it
	std::array<terraformer::location, 17> points;
	for(size_t k = 0; k != std::size(points); ++k)
	{
		auto const t = static_cast<float>(k)/static_cast<float>(std::size(points) - 1);
		points[k] = terraformer::location{} + p(t);
	}
	pretty::plot(points);

	constexpr terraformer::location point_to_query{0.8f, 0.2f, 0.0f};
	// Delta polynomial
	constexpr auto point_to_poly = p - terraformer::polynomial{point_to_query - terraformer::location{}};

	// Straight line approximation
	constexpr auto curve_start = terraformer::location{} + p(0.0f);
	constexpr auto curve_vector = terraformer::location{} + p(1.0f) - curve_start;
	constexpr auto seg_length = norm(curve_vector);
	constexpr auto p0_to_loc = point_to_query - curve_start;
	constexpr auto tangent = curve_vector/seg_length;
	auto t = inner_product(tangent, p0_to_loc)/seg_length;
	
	PRETTY_PRINT_EXPR(t);

	constexpr auto should_be_zero = multiply(
		point_to_poly,
		point_to_poly,
		[](auto a, auto b) {
			return inner_product(a, b);
		}
	).derivative();

	constexpr auto should_be_zero_deriv = should_be_zero.derivative();

	for(size_t k = 0; k != 16; ++k)
	{
		t = t - should_be_zero(t)/should_be_zero_deriv(t);
		PRETTY_PRINT_EXPR(t);
		PRETTY_PRINT_EXPR(should_be_zero(t));
		PRETTY_PRINT_EXPR(p(t));
	}
}