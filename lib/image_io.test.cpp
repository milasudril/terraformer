//@	{
//@	 "target":{"name":"image_io.test"}
//@	}

#include "./image_io.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(LoadStoreLoad)
{
	auto image_a = load(Empty<Image>{}, "test_pattern/test_pattern.exr");
	auto id_string = std::string{MAIKE_BUILDINFO_TARGETDIR "/"};
	id_string += std::to_string(MAIKE_TASKID);
	store(image_a, id_string.c_str());
	auto image_b = load(Empty<Image>{}, id_string.c_str());
	EXPECT_EQ((std::ranges::equal(image_a.pixels(), image_b.pixels(), [](auto a, auto b) {
		return a.red() == b.red() && a.green() == b.green() && a.blue() == b.blue()
		       && a.alpha() == b.alpha();
	})), true);
}