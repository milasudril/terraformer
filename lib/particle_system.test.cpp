//@	{"target":{"name":"particle_system.test.cpp"}}

#include "./particle_system.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_particle_system_init)
{
	terraformer::particle_system const ps{3, [](size_t k) {
		auto const progress = static_cast<float>(k) + 0.5f;
		return terraformer::particle{
			.r = terraformer::location{progress, 2.0f*progress, 3.0f*progress},
			.v = terraformer::displacement{4.0f*progress, 5.0f*progress, 6.0f*progress},
			.integ_distance = float{7.0f*progress},
			.integ_heading = geosimd::turn_angle{geosimd::turns{8.0f*progress}}
		};
	}};

	EXPECT_EQ(std::size(ps), 3);

	for(size_t k = 0; k != std::size(ps); ++k)
	{
		auto const progress = static_cast<float>(k) + 0.5f;

		terraformer::particle  const p{
			.r = terraformer::location{progress, 2.0f*progress, 3.0f*progress},
			.v = terraformer::displacement{4.0f*progress, 5.0f*progress, 6.0f*progress},
			.integ_distance = float{7.0f*progress},
			.integ_heading = geosimd::turn_angle{geosimd::turns{8.0f*progress}}
		};

		EXPECT_EQ(ps.locations(k), p.r);
		EXPECT_EQ(ps.velocities(k), p.v);
		EXPECT_EQ(ps.integ_distances(k), p.integ_distance);
		EXPECT_EQ(ps.integ_headings(k), p.integ_heading);

		auto const p_stored = ps.get_particle(k);

		EXPECT_EQ(p_stored.r, p.r);
		EXPECT_EQ(p_stored.v, p.v);
		EXPECT_EQ(p_stored.integ_distance, p.integ_distance);
		EXPECT_EQ(p_stored.integ_heading, p.integ_heading);
	}

	{
		auto const loc = ps.locations();
		EXPECT_EQ(std::size(loc), std::size(ps));

		for(size_t k = 0; k != std::size(loc); ++k)
		{
			auto const progress = static_cast<float>(k) + 0.5f;
			terraformer::location const r{progress, 2.0f*progress, 3.0f*progress};
			EXPECT_EQ(loc[k], r);
		}
	}

	{
		auto const vel = ps.velocities();
		EXPECT_EQ(std::size(vel), std::size(ps));

		for(size_t k = 0; k != std::size(vel); ++k)
		{
			auto const progress = static_cast<float>(k) + 0.5f;
			terraformer::displacement const v{4.0f*progress, 5.0f*progress, 6.0f*progress};
			EXPECT_EQ(vel[k], v);
		}
	}

	{
		auto const d = ps.integ_distances();
		EXPECT_EQ(std::size(d), std::size(ps));

		for(size_t k = 0; k != std::size(d); ++k)
		{
			auto const progress = static_cast<float>(k) + 0.5f;
			EXPECT_EQ(d[k], 7.0f*progress);
		}
	}

	{
		auto const theta = ps.integ_headings();
		EXPECT_EQ(std::size(theta), std::size(ps));

		for(size_t k = 0; k != std::size(theta); ++k)
		{
			auto const progress = static_cast<float>(k) + 0.5f;
			geosimd::turn_angle const val{geosimd::turns{8.0f*progress}};
			EXPECT_EQ(theta[k], val);
		}
	}
}