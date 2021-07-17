//@	{"target":{"name":"main.o"}}

#include "lib/image.hpp"
#include "lib/image_io.hpp"

void debug(GrayscaleImage const& img)
{
	Image img_out{img.width(), img.height()};
	transform(img.pixels(), img_out.pixels(), [](auto, auto, auto v){ return RgbaPixel{v,v,v, 1.0f}; });
	store(img_out, "/dev/shm/slask.exr");
}

int main()
{
	GrayscaleImage img{1024, 512};

	debug(img);
}