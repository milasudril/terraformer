//@	{"target":{"name":"main.o"}}

#include "lib/image.hpp"
#include "lib/image_io.hpp"
#include "lib/mathutils.hpp"
#include "lib/tagged_type.hpp"

#include <pcg_random.hpp>

#include <numbers>
#include <random>

constexpr float MeterXY = 1.0f/128.0f;

constexpr float DomainWidth  = 131072.0f*MeterXY;
constexpr float DomainHeight = 6.0f*16384.0f*MeterXY;
constexpr float Level0SegLength = 16.0f*MeterXY;

using RngType = pcg32;

class XYLocGenerator
{
	enum class ParamId:int{DirDecayRateId, SegLengthId, SegLengthDecayRateId, LocationDecayRateId};

public:
	using DirDecayRate = TaggedType<float, ParamId::DirDecayRateId>;
	using SegLength = TaggedType<float, ParamId::SegLengthId>;
	using SegLengthDecayRate = TaggedType<float, ParamId::SegLengthDecayRateId>;
	using LocDecayRate = TaggedType<float, ParamId::LocationDecayRateId>;

	explicit XYLocGenerator(SegLength seg_length,
                            LocDecayRate loc_decay_rate,
                            DirDecayRate dir_decay_rate,
                            SegLengthDecayRate seg_length_decay_rate):
		m_loc{0.0f, 0.0f},
		m_dir{0.0f},
		m_seg_length{seg_length},
		m_loc_decay_rate{loc_decay_rate},
		m_dir_decay_rate{dir_decay_rate},
		m_seg_length_decay_rate{std::exp(-seg_length_decay_rate)}
	{}

	Point<float> operator()(RngType& rng)
	{
		auto const ret = m_loc;

		auto const angle_range = 1.0f*std::numbers::pi_v<float>/24.0f;
		auto angle_dist = std::uniform_real_distribution{-angle_range, angle_range};

		m_loc += m_seg_length*Vector{cos(m_dir), -sin(m_dir)}
			 + Vector{0.0f, -m_loc_decay_rate*m_loc.y()};
		m_seg_length *= m_seg_length_decay_rate;
		m_dir += angle_dist(rng) - m_dir_decay_rate*m_dir;

		return ret;
	}

	Point<float> location() const
	{ return m_loc; }

	XYLocGenerator& location(Point<float> loc)
	{
		m_loc = loc;
		return *this;
	}

private:
	Point<float> m_loc;
	float m_dir;

	float m_seg_length;
	LocDecayRate m_loc_decay_rate;
	DirDecayRate m_dir_decay_rate;
	SegLengthDecayRate m_seg_length_decay_rate;
};

class ElevationGenerator
{
	enum class ParamId:int{ElevDecayRateId};
public:
	using DecayRate = TaggedType<float, ParamId::ElevDecayRateId>;

	explicit ElevationGenerator(DecayRate elev_decay_rate):
		m_z{1.0f},
		m_elev_decay_rate{elev_decay_rate}
	{}

	float operator()(RngType& rng)
	{
		auto ret = m_z;
		auto z_dist = std::uniform_real_distribution{-1.0f, 1.0f};
		m_z += z_dist(rng) - m_elev_decay_rate*m_z;
		return ret;
	}

private:
	float m_z;
	DecayRate m_elev_decay_rate;
};

class RidgeGenerator
{
public:
	explicit RidgeGenerator(Extents<Image::IndexType> extents):
		m_xy_gen{XYLocGenerator::SegLength{Level0SegLength},
			XYLocGenerator::LocDecayRate{1.0f/384.0f},
			XYLocGenerator::DirDecayRate{1.0f/192.0f},
			XYLocGenerator::SegLengthDecayRate{0.0f}},
		m_z_gen{ElevationGenerator::DecayRate{1.0f/64.0f}},
		m_extents{static_cast<float>(extents.width()), 0.5f*extents.depth()}
	{
	}

	PolygonChain<float> operator()(RngType& rng)
	{
		{
			auto const loc = m_xy_gen.location();
			m_xy_gen.location(Point{0.0f, loc.y(), loc.z()});
		}
		auto offset = Vector{0.0f, m_extents.depth(), 1.0f};
		auto const Z = Vector{0.0f, 0.0f, 1.0f};

 		PolygonChain ret{m_xy_gen(rng) + m_z_gen(rng)*Z + offset, m_xy_gen(rng) + m_z_gen(rng)*Z + offset};
		auto loc = m_xy_gen(rng) + offset;
		while(loc.x() < m_extents.width())
		{
			ret.append(loc);
			loc = m_xy_gen(rng) + m_z_gen(rng)*Z + offset;
		}
		ret.append(loc);
		return ret;
	}

private:
	XYLocGenerator m_xy_gen;
	ElevationGenerator m_z_gen;
	Extents<float> m_extents;
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
		auto int_pos = vector_cast<uint32_t>(pos + Vector{0.5f, 0.5f, 0.5f});
		if(within(xy(int_pos), img.extents()))
		{
			img(int_pos.x(), int_pos.y()) = pos.z();
		}
	}
}

void draw(PolygonChain<float> const& polychain, GrayscaleImage& img_out)
{
	auto verts = polychain.vertices();
	adj_for_each(std::begin(verts), std::end(verts), [&img = img_out](auto from, auto to){
		draw(LineSegment{from, to}, img);
	});
}

void normalize_elevation(std::span<Point<float>> points)
{
	auto min_elevation = [](auto a, auto b){ return a.z() < b.z(); };
	auto min = std::ranges::min_element(points, min_elevation)->z();
	auto max = std::ranges::max_element(points, min_elevation)->z();
	auto range = max - min;;
	std::ranges::for_each(points, [min, range](auto& p){
		p = Point{p.x(), p.y(), (p.z() - min)/range};
	});
}

void set_horizontal_boundary(Span2d<float> dest, uint32_t y, float val)
{
	// Dirichlet
	for(uint32_t x = 0u; x != dest.width(); ++x)
	{
		dest(x, y) = val;
	}
}

void set_vertical_boundaries(Span2d<float> dest)
{
	//	Neumann
	for(uint32_t y = 0u; y != dest.height(); ++y)
	{
		dest(0, y) = dest(1, y);
		dest(dest.width() - 1, y) = dest(dest.width() - 2, y);
	}
}

void diffuse(Span2d<float const> src, Span2d<float> dest)
{
	std::fill(std::begin(dest), std::end(dest), 0.0f);
	for(uint32_t y = 1u; y != src.height() - 1; ++y)
	{
		for(uint32_t x = 1u; x != src.width() - 1; ++x)
		{
			auto const d_xx = src(x + 1, y) - 2.0f*src(x, y) + src(x - 1, y);
			auto const d_yy = src(x, y + 1) - 2.0f*src(x, y) + src(x , y - 1);

			auto const laplace = d_xx + d_yy;

			dest(x, y) += src(x, y) + 0.125f*laplace;
		}
	}
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
	RngType rng;

	GrayscaleImage img_a{static_cast<uint32_t>(DomainWidth), static_cast<uint32_t>(DomainHeight)};
	std::fill(std::begin(img_a.pixels()), std::end(img_a.pixels()), 0.0f);
	auto img_b = img_a;
	RidgeGenerator make_ridge{img_a.extents()};
	std::reference_wrapper in{img_a};
	std::reference_wrapper out{img_b};

//	draw(PolygonChain{Point{0.0f, DomainHeight/3.0f, 1.0f}, Point{DomainWidth, DomainHeight/3.0f, 1.0f}}, in.get());
//	draw(PolygonChain{Point{0.0f, 2.0f*DomainHeight/3.0f, 1.0f}, Point{DomainWidth, 2.0f*DomainHeight/3.0f, 1.0f}}, in.get());

	for(int k = 0; k < 1; ++k)
	{

		auto ridge = make_ridge(rng);
		normalize_elevation(ridge.vertices());
		std::ranges::for_each(ridge.vertices(), [](auto& val){
			val = Origin<float> + scale(val - Origin<float>, Vector{1.0f, 1.0f, 3072.0f}) + 1024.0f*Z<float>;
		});

		auto upper = make_ridge(rng);
		normalize_elevation(upper.vertices());
		std::ranges::for_each(upper.vertices(), [](auto& val){
			val = Origin<float> + scale(val - Origin<float>, Vector{1.0f, 1.0f, 128.0f})
				+ Vector{0.0f, -1.0f*DomainHeight/6.0f, 640.0f};
		});

		auto lower = make_ridge(rng);
		normalize_elevation(lower.vertices());
		std::ranges::for_each(lower.vertices(), [](auto& val){
			val = Origin<float> + scale(val - Origin<float>, Vector{1.0f, 1.0f, 64.0f})
				+ Vector{0.0f, 1.0f*DomainHeight/6.0f, 320.0f};
		});

		std::ranges::for_each(ridge.vertices(), [&img = in.get()](auto& val){
			auto const int_pos = vector_cast<uint32_t>(val + Vector{0.5f, 0.5f, 0.5f});
			if(within(xy(int_pos), img.extents()))
			{
				val += img(int_pos.x(), int_pos.y())*Z<float>;
			}
		});
#if 0
		draw(ridge, in.get());
		draw(upper, in.get());
		draw(lower, in.get());
//		puts("Tock");
#else
		for(int l = 0; l < 65536; ++l)
		{
			diffuse(in.get().pixels(), out.get().pixels());
			draw(ridge, out.get());
			draw(upper, out.get());
			draw(lower, out.get());
			set_horizontal_boundary(out.get().pixels(), 0, 512.0f);
			set_horizontal_boundary(out.get().pixels(), img_a.height() - 1, 0);
			set_vertical_boundaries(out.get());
			std::swap(in, out);
			if(l%128 == 0)
			{
				printf("%d applying diffusion\n", l);
			}
		}
#endif
		printf("Next iter %d\n", k);
	}
	debug(in);
}