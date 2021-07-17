//@	{"target":{"name":"main.o"}}

#include "lib/image.hpp"
#include "lib/image_io.hpp"
#include "lib/mathutils.hpp"

#include <pcg_random.hpp>

#include <numbers>
#include <random>

PolygonChain<float> make_ridge(Extents<Image::IndexType> ex, pcg32& rng)
{
	auto const ex_new = Extents{static_cast<float>(ex.width()), 0.5f*ex.depth()};
	auto const arc = make_arc(ex_new);
	auto const s = length(arc);
	auto const dθ_max = 256.0f*static_cast<float>(arc.angle/s);
//	printf("%.8g\n", dθ_max/std::numbers::pi_v<float>);
	auto angle_dist = std::uniform_real_distribution{-dθ_max, dθ_max};
	Point const pos_init{0.0f, ex_new.depth(), 1.0f};
	auto θ = 0.0f;
	θ += angle_dist(rng);
	auto pos = pos_init + Vector{std::cos(θ), -std::sin(θ), 0.0f};
	PolygonChain ret{pos_init, pos};
	for(int k = 1; k < static_cast<int>(s); ++k)
	{
		pos += Vector{std::cos(θ), -std::sin(θ), 0.0f};;
		θ += angle_dist(rng);
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
	pcg32 rng;

	GrayscaleImage img{1024, 512};
	auto ridge = make_ridge(img.extents(), rng);

	std::ranges::for_each(ridge.vertices(), [&img](auto pos) {
		printf("%.8g %.8g\n", pos.x(), pos.y());
	});

	debug(img);
}