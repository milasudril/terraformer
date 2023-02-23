#ifndef TERRAFORMER_GEOMODELS_SUNDIRECTION_HPP
#define TERRAFORMER_GEOMODELS_SUNDIRECTION_HPP

#include "lib/common/spaces.hpp"
#include <geosimd/mat_4x4.hpp>

namespace terraformer
{
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
	//	printf("r = %s\n", to_string(r).c_str());
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