//@	{"target":{"name":"spaces.test"}}

#include "./spaces.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_lib_spaces_type_properties)
{
	EXPECT_EQ(geosimd::point<terraformer::location>, true);
	EXPECT_EQ(geosimd::vector<terraformer::location>, false);
	EXPECT_EQ(terraformer::location::size(), 3);

	EXPECT_EQ(geosimd::vector<terraformer::displacement>, true);
	EXPECT_EQ(terraformer::displacement::size(), 3);

	EXPECT_EQ(geosimd::vector<terraformer::direction>, false);
	EXPECT_EQ(terraformer::direction::size(), 3);
}