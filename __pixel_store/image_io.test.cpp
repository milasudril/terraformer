//@	{
//@	 "target":{"name":"image_io.test"}
//@	}

#include "./image_io.hpp"

#define STR(a) str(a)
#define str(a) #a

#include <cassert>

namespace Testcases
{
	void texpainterPixelStoreLoadStoreLoad()
	{
		auto image_a =
		    load(Enum::Empty<PixelStore::Image>{}, "test_pattern/test_pattern.exr");
		store(image_a, STR(MAIKE_TARGET_DIRECTORY) "/testimg.exr");
		auto image_b = load(Enum::Empty<PixelStore::Image>{},
		                    STR(MAIKE_TARGET_DIRECTORY) "/testimg.exr");
		assert((std::ranges::equal(image_a.pixels(), image_b.pixels(), [](auto a, auto b) {
			return a.red() == b.red() && a.green() == b.green() && a.blue() == b.blue()
			       && a.alpha() == b.alpha();
		})));
	}
}

int main()
{
	Testcases::texpainterPixelStoreLoadStoreLoad();
	return 0;
}