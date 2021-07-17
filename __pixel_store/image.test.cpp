//@	{"targets":[{"name":"image.test", "type":"application", "autorun":1}]}

#include "./image.hpp"

#include <cassert>
#include <vector>
#include <functional>
#include <cstring>

namespace Testcases
{
	void texpainterImageCreateFromDataBlock()
	{
		PixelStore::Image img{3u, 2u};
		assert(img.width() == 3);
		assert(img.height() == 2);
		assert(area(img) == 6);

		auto ptr   = img.pixels().begin();
		*(ptr + 0) = PixelStore::red();
		*(ptr + 1) = PixelStore::green();
		*(ptr + 2) = PixelStore::blue();
		*(ptr + 3) = PixelStore::cyan();
		*(ptr + 4) = PixelStore::magenta();
		*(ptr + 5) = PixelStore::yellow();

		assert(distanceSquared(img(0, 0), PixelStore::red()) == 0.0f);
		assert(distanceSquared(img(1, 0), PixelStore::green()) == 0.0f);
		assert(distanceSquared(img(2, 0), PixelStore::blue()) == 0.0f);
		assert(distanceSquared(img(0, 1), PixelStore::cyan()) == 0.0f);
		assert(distanceSquared(img(1, 1), PixelStore::magenta()) == 0.0f);
		assert(distanceSquared(img(2, 1), PixelStore::yellow()) == 0.0f);
	}
	void texpainterImageCreateEmpty()
	{
		PixelStore::Image img{3u, 2u};
		assert(img.width() == 3);
		assert(img.height() == 2);
	}
}

int main()
{
	Testcases::texpainterImageCreateFromDataBlock();
	Testcases::texpainterImageCreateEmpty();
}