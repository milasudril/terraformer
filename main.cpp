//@	{"target":{"name":"main.o"}}

#include "lib/image.hpp"
#include "lib/image_io.hpp"
#include "lib/mathutils.hpp"

#include <pcg_random.hpp>

#include <numbers>
#include <random>

PolygonChain<float> make_ridge(Extents<Image::IndexType> ex, pcg32& rng)
{
	std::normal_distribution<float> n{0.5f*ex.depth(), 0.125f*ex.depth()};
	auto const ex_new = Extents{static_cast<float>(ex.width()), n(rng)};
	auto angle_dist = std::uniform_real_distribution{-0.5f*std::numbers::pi_v<float>, 0.5f*std::numbers::pi_v<float>};
	auto dz_dist = std::uniform_real_distribution{0.0f, 1.0f};
	Point const pos_init{0.0f, ex_new.depth(), 1.0f};
	auto θ = 0.0f;
	θ += angle_dist(rng);
	auto pos = pos_init + 16.0f*Vector{std::cos(θ), -std::sin(θ), 0.0f};
	PolygonChain ret{pos_init, pos};
	while(pos.x() < ex_new.width())
	{
		pos += 16.0f*Vector{std::cos(θ), -std::sin(θ), dz_dist(rng)};
		θ += angle_dist(rng) - 0.5f*θ;
		ret.append(pos);
	}

	return ret;
}

void draw(LineSegment<float> const& l, GrayscaleImage& img)
{
	auto const v = l.to - l.from;
	auto const dist_xy = length(xy(v));
	auto const 𝐭 = (1.0f/dist_xy) * v;
	for(int k = 0; k <= static_cast<int>(dist_xy); ++k)
	{
		auto const t = static_cast<float>(k);
		auto const pos = l.from + t*𝐭;
		auto int_pos = vector_cast<uint32_t>(pos);
		if(within(xy(int_pos), img.extents()))
		{ img(int_pos.x(), int_pos.y()) += pos.z(); }
	}
}

void draw(PolygonChain<float> const& polychain, GrayscaleImage& img_out)
{
	auto verts = polychain.vertices();
	adj_for_each(std::begin(verts), std::end(verts), [&img = img_out](auto from, auto to){
		draw(LineSegment{from, to}, img);
	});
}

void debug(GrayscaleImage const& img)
{
	auto ptr = img.pixels().data();
	auto v = *std::max_element(ptr, ptr + area(img));
	Image img_out{img.width(), img.height()};
	transform(img.pixels(), img_out.pixels(), [v_max =v](auto, auto, auto v) {
		v/=v_max;
		return RgbaPixel{v, v, v, 1.0f};
	});
	store(img_out, "/dev/shm/slask.exr");
}



int main()
{
	pcg32 rng;

	GrayscaleImage img{1024, 512};
	for(int k = 0; k < 16; ++k)
	{
		auto ridge = make_ridge(img.extents(), rng);
		draw(ridge, img);
	}
	debug(img);

//	std::ranges::for_each(ridge.vertices(), [&img](auto pos) {
//		printf("%.8g %.8g\n", pos.x(), pos.y());
//	});

	debug(img);
}