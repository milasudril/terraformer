#ifndef TERRAFORMER_GEOMODELS_COORDINATETRANSFORMS_HPP
#define TERRAFORMER_GEOMODELS_COORDINATETRANSFORMS_HPP

#include "./year.hpp"

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"

#include <geosimd/mat_4x4.hpp>

namespace terraformer
{
	inline auto to_map_location(pixel_coordinates loc, span_2d_extents domain_size, double pixel_size)
	{
		hires_displacement const r{static_cast<double>(loc.x), static_cast<double>(loc.y), 0.0};
		hires_displacement const size_vec{
			static_cast<double>(domain_size.width),
			static_cast<double>(domain_size.height),
			0.0
		};

		return hires_origin + pixel_size*(r - 0.5*size_vec + hires_displacement{0.5, 0.5, 0.0});
	}

	inline auto to_colatitude(geosimd::turn_angle latitude)
	{
		return geosimd::rotation_angle{0x4000'0000} - latitude;
	}

	struct longcolat
	{
		geosimd::rotation_angle longitude;
		geosimd::rotation_angle colatitude;
	};

	inline auto to_longcolat(hires_location loc,
		double planet_radius,
		geosimd::rotation_angle colat_offset)
	{
		auto const theta = colat_offset + geosimd::turn_angle{geosimd::rad{loc[1]/planet_radius}};
		geosimd::rotation_angle const phi{geosimd::rad{loc[0]/(planet_radius*sin(theta))}};

		return longcolat{
			.longitude = phi,
			.colatitude = theta
		};
	}

	inline auto planet_location(year t, double distance_to_sun)
	{
		auto const cs = cossin<double>(geosimd::turn_angle{geosimd::turns{t.value()}});
		return hires_origin + distance_to_sun*hires_direction{cs, geosimd::dimension_tag<2>{}};
	}

	inline auto planet_rotation(geosimd::turn_angle spin_angle, geosimd::turn_angle tilt_angle)
	{
		geosimd::rotation<hires_geom_space> ret{};
		ret.push(tilt_angle, geosimd::dimension_tag<1>{})
			.push(spin_angle, geosimd::dimension_tag<2>{});
		return ret;
	}

	template<class T>
	concept planetary_tilt_modulation = requires(T x, year t)
	{
		{x(t)} -> std::same_as<geosimd::turn_angle>;
	};

	template<planetary_tilt_modulation TiltModulation>
	inline auto planet_rotation(year t, double spin_freq, TiltModulation&& tilt_mod)
	{
		geosimd::turn_angle const spin_angle{geosimd::turns{spin_freq*t.value()}};
		return planet_rotation(spin_angle, std::forward<TiltModulation>(tilt_mod)(t));
	}
}

#endif