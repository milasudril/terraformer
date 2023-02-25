#ifndef TERRAFORMER_GEOMODELS_SUNDIRECTION_HPP
#define TERRAFORMER_GEOMODELS_SUNDIRECTION_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"

#include <geosimd/mat_4x4.hpp>

namespace terraformer
{
	struct longcolat
	{
		geosimd::rotation_angle longitude;
		geosimd::rotation_angle colatitude;
	};

	inline auto to_longcolat(pixel_coordinates loc,
		double pixel_size,
		double planet_radius,
		geosimd::rotation_angle top_colat)
	{
		auto const theta = top_colat
			+ geosimd::turn_angle{
				geosimd::rad{(static_cast<double>(loc.y) + 0.5)*pixel_size/planet_radius}
			};
		geosimd::rotation_angle const phi{
			geosimd::rad{(static_cast<double>(loc.x) + 0.5)*pixel_size/(planet_radius*sin(theta))}
		};

		return longcolat{
			.longitude = phi,
			.colatitude = theta
		};
	}

#if 0
	inline auto planet_location(geosimd::turn_angle year, double distance_to_sun)
	{
		auto const cs = cossin<double>(year);
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
	concept planetary_tilt_modulation = requires(T x, geosimd::turn_angle t)
	{
		{x(t)} -> std::same_as<geosimd::rotation_angle>;
	};

	template<planetary_tilt_modulation TiltModulation>
	inline auto planet_rotation(geosimd::turn_angle year, double spin_freq, TiltModulation&& tilt_mod)
	{
		return planet_rotation(spin_freq*year, std::forward<TiltModulation>(tilt_mod)(year));
	}
#endif

	inline auto local_sun_direction(hires_location planet_location,
		geosimd::rotation<hires_geom_space> const& planet_rotation,
		geosimd::rotation_angle longitude,
		geosimd::rotation_angle colatitude)
	{
		auto const longcs = cossin<double>(longitude);
		auto const colatcs = cossin<double>(colatitude);

		auto const x = longcs.cos() * colatcs.sin();
		auto const y = longcs.sin() * colatcs.sin();
		auto const z = colatcs.cos();

		auto const r = hires_displacement{x, y, z}.apply(planet_rotation);
		auto const loc_observer = planet_location + r;
		hires_displacement const sun_dir_xyz{hires_origin - loc_observer};

		hires_displacement const dx{
			-longcs.sin(),
			colatcs.cos()*longcs.cos(),
			colatcs.sin()*longcs.cos()
		};

		hires_displacement const dy{
			longcs.cos(),
			colatcs.cos()*longcs.sin(),
			colatcs.sin()*longcs.sin()
		};

		hires_displacement const dz{
			0.0,
			-colatcs.sin(),
			colatcs.cos()
		};

		geosimd::mat_4x4 const m{
			std::array<geosimd::vec_t<double, 4>, 4>{
				dx.get(),
				dy.get(),
				dz.get(),
				geosimd::vec_t{0.0, 0.0, 0.0, 1.0}
		}};

		return hires_direction{
			hires_displacement{
				m*inverted(planet_rotation).get()*sun_dir_xyz.get().get()
			}
		};
	}
}

#endif