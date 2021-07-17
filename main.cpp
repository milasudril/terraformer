//@	{"target":{"name":"main.o"}}

#include "lib/image.hpp"
#include "lib/image_io.hpp"
#include "lib/mathutils.hpp"

#include <pcg_random.hpp>


PolygonChain<float> make_ridge(Extents<Image::IndexType> ex)
{
	auto const ex_new = Extents{static_cast<float>(ex.width()), 0.5f*ex.depth()};
	auto const arc = make_arc(ex_new);
	auto const s = length(arc);
	auto const dθ = static_cast<float>(arc.angle/s);
	Point const pos_init{0.0f, ex_new.depth(), 1.0f};
	auto pos = pos_init + Vector{std::cos(dθ), -std::sin(dθ), 0.0f};;
	PolygonChain ret{pos_init, pos};
	for(int k = 1; k < static_cast<int>(s); ++k)
	{
		pos += Vector{std::cos(k*dθ), -std::sin(k*dθ), 0.0f};;
		ret.append(pos);
	}

	return ret;
}

void debug(GrayscaleImage const& img)
{
	Image img_out{img.width(), img.height()};
	transform(img.pixels(), img_out.pixels(), [](auto, auto, auto v){ return RgbaPixel{v,v,v, 1.0f}; });
	store(img_out, "/dev/shm/slask.exr");
}



int main()
{
	GrayscaleImage img{1024, 512};
	auto ridge = make_ridge(img.extents());

	std::ranges::for_each(ridge.vertices(), [&img](auto pos) {
		printf("%.8g %.8g\n", pos.x(), pos.y());
	});

	debug(img);
}