//@	{"target":{"name":"main.o"}}

#include "lib/image.hpp"
#include "lib/image_io.hpp"
#include "lib/mathutils.hpp"

#include <pcg_random.hpp>

#include <numbers>
#include <random>

class RidgeGenerator
{
public:
	explicit RidgeGenerator(pcg32& rng, Extents<Image::IndexType> extents):
		m_rng{rng},
		m_extents{static_cast<float>(extents.width()), 0.5f*extents.depth()},
		pos{0.0f, m_extents.depth(), 1.0f},
		θ{0.0f},
		dz{0.0f}
	{
	}

	PolygonChain<float> operator()()
	{
		auto angle_dist = std::uniform_real_distribution{-0.5f*std::numbers::pi_v<float>, 0.5f*std::numbers::pi_v<float>};
		auto const pos_init = pos;
		θ += angle_dist(m_rng.get()) - 0.5f*θ;
		pos += 16.0f*Vector{std::cos(θ), -std::sin(θ), 0.0f} + Vector{0.0f, 0.125f*(m_extents.depth() - pos.y()), 0.0f};
		θ += angle_dist(m_rng.get()) - 0.5f*θ;
 		PolygonChain ret{pos_init, pos};
		while(pos.x() < m_extents.width())
		{
			pos += 16.0f*Vector{std::cos(θ), -std::sin(θ), 0.0f} + Vector{0.0f, 0.125f*(m_extents.depth() - pos.y()), 0.0f};
			θ += angle_dist(m_rng.get()) - 0.5f*θ;
			ret.append(pos);
		}
		return ret;
	}

private:
	std::reference_wrapper<pcg32> m_rng;
	Extents<float> m_extents;
	Point<float> pos;
	float θ;
	float dz;
};

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
	RidgeGenerator make_ridge{rng, img.extents()};
	for(int k = 0; k < 1; ++k)
	{
		auto ridge = make_ridge();
		draw(ridge, img);
	}
	debug(img);
}