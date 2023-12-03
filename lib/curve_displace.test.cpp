//@	{"target":{"name": "curve_displace.test"}}

#include "./curve_displace.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_curve_vertex_normal_from_curvature_to_circle_in_plane)
{
	std::array<terraformer::location, 16> locs;  // Using 2^k here to make native mod sane
	for(size_t k = 0; k != std::size(locs); ++k)
	{
			auto const theta = static_cast<float>(k)*(2.0f*std::numbers::pi_v<float>)/static_cast<float>(std::size(locs));
			locs[k] = terraformer::location{std::cos(theta), std::sin(theta), 0.0f};
	}

	for(size_t k = 0; k != std::size(locs); ++k)
	{
		auto const a = locs[(k - 1)%std::size(locs)];
		auto const b = locs[k];
		auto const c = locs[(k + 1)%std::size(locs)];
		auto const n = terraformer::curve_vertex_normal_from_curvature(a, b, c);

		auto const dir = b - terraformer::location{0.0f, 0.0f, 0.0f};
		EXPECT_LT(norm(dir - n.get()), 1.0e-6f);
	}
}

TESTCASE(terraformer_curve_vertex_normal_from_curvature_to_circle_rotated_arround_y)
{
	std::array<terraformer::location, 16> locs;  // Using 2^k here to make native mod sane
	for(size_t k = 0; k != std::size(locs); ++k)
	{
		auto const theta = static_cast<float>(k)*(2.0f*std::numbers::pi_v<float>)/static_cast<float>(std::size(locs));
		auto const d = terraformer::displacement{std::cos(theta), std::sin(theta), 0.0f}
			.apply(
				terraformer::rotation{
					geosimd::rotation_angle{geosimd::turns{0.125}},
					geosimd::dimension_tag<1>{}
				}
			);
		locs[k] = terraformer::location{0.0f, 0.0f, 0.0f} + d;
	}

	for(size_t k = 0; k != std::size(locs); ++k)
	{
		auto const a = locs[(k - 1)%std::size(locs)];
		auto const b = locs[k];
		auto const c = locs[(k + 1)%std::size(locs)];
		auto const n = terraformer::curve_vertex_normal_from_curvature(a, b, c);

		EXPECT_NE(b[2], 0.0f);

		auto const dir = b - terraformer::location{0.0f, 0.0f, 0.0f};
		EXPECT_LT(norm(dir - n.get()), 1.0e-6f);
	}
}

TESTCASE(terraformer_curve_vertex_normal_from_curvature)
{
	std::array<terraformer::location, 16> locs;
	for(size_t k = 0; k != std::size(locs); ++k)
	{
		auto const theta = static_cast<float>(k)*(2.0f*std::numbers::pi_v<float>)/static_cast<float>(std::size(locs));
		locs[k] = terraformer::location{theta, std::sin(theta), 0.0f};
	}

	for(size_t k = 1; k != std::size(locs) - 1; ++k)
	{
		auto const a = locs[k - 1];
		auto const b = locs[k];
		auto const c = locs[k + 1];
		auto const n = terraformer::curve_vertex_normal_from_curvature(a, b, c);

		if(locs[k][1] > 0.0f)
		{ EXPECT_GT(n[1], 0.0f); }

		if(locs[k][1] < 0.0f)
		{ EXPECT_LT(n[1], 0.0f); }
	}
}

TESTCASE(terraformer_curve_vertex_normal_from_projection)
{
	std::array<terraformer::location, 16> locs;
	for(size_t k = 0; k != std::size(locs); ++k)
	{
		auto const theta = static_cast<float>(k)*(2.0f*std::numbers::pi_v<float>)/static_cast<float>(std::size(locs));
		locs[k] = terraformer::location{theta, std::sin(theta), 0.0f};
	}

	for(size_t k = 1; k != std::size(locs) - 1; ++k)
	{
		auto const a = locs[k - 1];
		auto const b = locs[k];
		auto const c = locs[k + 1];
		auto const n = terraformer::curve_vertex_normal_from_projection(a, b, c, terraformer::displacement{0.0f, 0.0f, -1.0f});

		EXPECT_LT(n[1], 0.0f);
	}
}