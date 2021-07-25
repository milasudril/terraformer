//@	{"target":{"name":"main.o"}}

#include "lib/image.hpp"
#include "lib/image_io.hpp"
#include "lib/mathutils.hpp"
#include "lib/tagged_type.hpp"

#include <pcg_random.hpp>

#include <numbers>
#include <random>

constexpr float MeterXY = 1.0f/64.0f;

constexpr float DomainWidth  = 65535.0f*MeterXY;
constexpr float DomainHeight = 49152.0f*MeterXY;
constexpr float Level0SegLength = 16.0f*MeterXY;
constexpr float Level0BranchDistance = 4096.0f*MeterXY/3.0f;

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

	XYLocGenerator& direction(float val)
	{
		m_dir = val;
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

void fill(std::span<Point<float> const> sources, GrayscaleImage& img)
{
	for(uint32_t y = 0; y != img.height(); ++y)
	{
		for(uint32_t x = 0; x != img.width(); ++x)
		{
			auto const loc = Point{static_cast<float>(x), static_cast<float>(y)};
			auto const min = *std::ranges::min_element(sources, [loc](auto val_a, auto val_b) {
				return distance_squared(loc, val_a) < distance_squared(loc, val_b);
			});

			img(x, y) += std::exp2(-distance(loc, min)/(2048.0f*MeterXY));
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

void debug(GrayscaleImage const& img, char const* name)
{
	auto ptr = img.pixels().data();
	auto v = *std::max_element(ptr, ptr + area(img));
	Image img_out{img.width(), img.height()};
	transform(img.pixels(), img_out.pixels(), [v_max =v](auto, auto, auto v) {
		v/=v_max;
		return RgbaPixel{v, v, v, 1.0f};
	});
	store(img_out, name);
}

template<class T>
struct BoundaryInfo
{
	Vector<T> scale;
	Vector<T> offset;
};

PolygonChain<float> make_ridge(XYLocGenerator& get_location,
							   RngType& rng,
							   float length)
{
	{
		auto const loc = get_location.location();
		get_location.location(Point{0.0f, loc.y(), loc.z()});
	}
	constexpr auto offset = Z<float>;

	PolygonChain ret{get_location(rng) + offset, get_location(rng) + offset};
	auto loc = get_location(rng) + offset;
	while(loc.x() < length)
	{
		ret.append(loc);
		loc = get_location(rng) + offset;
	}
	ret.append(loc);
	return ret;
}

GrayscaleImage generate_envelope(XYLocGenerator& get_location,
							     RngType& rng,
								 Extents<float> extents,
								 std::span<BoundaryInfo<float> const> bi)
{
	GrayscaleImage img_a{static_cast<uint32_t>(extents.width()), static_cast<uint32_t>(extents.depth())};
	auto img_b = img_a;
	std::vector<PolygonChain<float>> ridges;
	ridges.reserve(std::size(bi));
	std::ranges::transform(bi, std::back_inserter(ridges),
				   [&get_location, &rng, length=extents.width()](auto const& val) {
		auto ret = make_ridge(get_location, rng, length);
		scale(ret, val.scale);
		translate(ret, val.offset);
		return ret;
	});

	std::reference_wrapper in{img_a};
	std::reference_wrapper out{img_b};

	for(size_t l = 0; l != (1 << 17); ++l)
	{
		diffuse(in.get().pixels(), out.get().pixels());
		std::ranges::for_each(ridges, [&out = out.get()](auto const& ridge) {
			draw(ridge, out);
		});

		set_horizontal_boundary(out.get().pixels(), 0, 512.0f);
		set_horizontal_boundary(out.get().pixels(), img_a.height() - 1, 0);
		set_vertical_boundaries(out.get());
		std::swap(in, out);
		if(l%128 == 0)
		{
			printf("%zu applying diffusion\n", l);
		}
	}

	return in.get();
}

std::vector<PolygonChain<float>> generate_extensions(PolygonChain<float> const& trunc,
	RngType& rng,
	float branch_distance = Level0BranchDistance,
	size_t level = 0)
{
	std::vector<PolygonChain<float>> ret;

	adj_for_each(std::begin(trunc.vertices()), std::end(trunc.vertices()), [
		gamma = std::gamma_distribution{3.0f, branch_distance},
		level,
		&rng,
		branch_dist = 0.0f,
		l = 0.0f,
		&ret](auto a, auto b) mutable {
		l += distance(a, b);
		if(l >= branch_dist)
		{
			branch_dist = gamma(rng);
			XYLocGenerator get_location
			{
				XYLocGenerator::SegLength{Level0SegLength},
				XYLocGenerator::LocDecayRate{1.0f/384.0f},
				XYLocGenerator::DirDecayRate{1.0f/192.0f},
				XYLocGenerator::SegLengthDecayRate{0.0f}
			};
			auto ridge = make_ridge(get_location, rng, 1.0f*l);
			auto const O = *std::begin(ridge.vertices());
			scale(ridge, Vector{0.5f, 0.5f, 0.0f}, O);
			auto const t = normalized(b - a);
			auto const n = Vector{t.y(), -t.x()};
			auto const m = midpoint(a, b);
			auto const dir = dot(n, b - m) < 0.0f ? 1.0f : -1.0f;
			transform(ridge, dir*t, dir*n, Z<float>, O);
			transform(ridge, -1.0f*Y<float>, 1.0f*X<float>, Z<float>, O);
			translate(ridge, m - O);
			if(level != 1)
			{
				auto tmp = generate_extensions(ridge, rng, 0.5f*gamma.beta(), level + 1);
				std::ranges::move(tmp, std::back_inserter(ret));
			}
 			ret.push_back(std::move(ridge));
			l = 0.0f;
		}
	});

	return ret;
}

int main()
{
	RngType rng;
	XYLocGenerator loc_gen
	{
		XYLocGenerator::SegLength{Level0SegLength},
		XYLocGenerator::LocDecayRate{1.0f/384.0f},
		XYLocGenerator::DirDecayRate{1.0f/192.0f},
		XYLocGenerator::SegLengthDecayRate{0.0f}
	};
#if 0
	{
		std::array<BoundaryInfo<float>, 3> higher_env_params
		{
			BoundaryInfo{Vector{1.0f, 1.0f, 1024.0f}, (DomainHeight/3.0f) * Y<float>},
			BoundaryInfo{Vector{1.0f, 1.0f, 5120.0f}, (DomainHeight/2.0f) * Y<float>},
			BoundaryInfo{Vector{1.0f, 1.0f, 512.0f},  (2.0f*DomainHeight/3.0f) * Y<float>}
		};
		auto higher_env = generate_envelope(loc_gen, rng, Extents{DomainWidth, DomainHeight}, higher_env_params);

		debug(higher_env, "/dev/shm/slask_a.exr");
	}

	{
		std::array<BoundaryInfo<float>, 3> lower_env_params
			{
				BoundaryInfo{Vector{1.0f, 1.0f, 768.0f}, (DomainHeight/3.0f) * Y<float>},
				BoundaryInfo{Vector{1.0f, 1.0f, 1024.0f}, (DomainHeight/2.0f) * Y<float>},
				BoundaryInfo{Vector{1.0f, 1.0f, 384.0f},  (2.0f*DomainHeight/3.0f) * Y<float>}
			};
		auto lower_env = generate_envelope(loc_gen, rng, Extents{DomainWidth, DomainHeight}, lower_env_params);
		debug(lower_env, "/dev/shm/slask_b.exr");
	}
#endif
	{
		auto img_a = GrayscaleImage{static_cast<uint32_t>(DomainWidth), static_cast<uint32_t>(DomainHeight)};


		auto a = make_ridge(loc_gen, rng, DomainWidth);
		translate(a, (DomainHeight/3.0f + DomainHeight/24.0f) * Y<float>);
		auto ext_a = generate_extensions(a, rng);


		auto b = make_ridge(loc_gen, rng, DomainWidth);
		translate(b, (DomainHeight/3.0f + 7.0f*DomainHeight/24.0f) * Y<float>);
		auto ext_b = generate_extensions(b, rng);

		std::vector<Point<float>> ridges;
		std::ranges::copy(a.vertices(), std::back_inserter(ridges));
		std::ranges::copy(b.vertices(), std::back_inserter(ridges));
		std::ranges::for_each(ext_a, [&ridges](auto const& val) {
			std::ranges::copy(val.vertices(), std::back_inserter(ridges));
		});
		std::ranges::for_each(ext_b, [&ridges](auto const& val) {
			std::ranges::copy(val.vertices(), std::back_inserter(ridges));
		});

		fill(ridges, img_a);



#if 1

#else
		std::ranges::for_each(a.vertices(), [](auto loc) mutable{
			printf("%.8g %.8g\n", loc.x(), loc.y());
		});
#endif
#if 0

		draw(a, img);
#endif
		debug(img_a, "/dev/shm/slask.exr");
	}

}
