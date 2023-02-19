#ifndef TERRAFORMER_GEOMODELS_SUNDIRECTION_HPP
#define TERRAFORMER_GEOMODELS_SUNDIRECTION_HPP

#include "lib/common/spaces.hpp"
#include <geosimd/mat_4x4.hpp>

namespace terraformer
{
	inline auto orbital_sun_direction(hires_location earth_loc,
		hires_displacement view_offset,
		hires_location sun_loc = hires_origin)
	{
		return hires_direction{sun_loc - (earth_loc + view_offset)};
	}

	inline auto local_sun_direction(hires_location earth_loc,
		hires_displacement view_offset,
		hires_location sun_loc = hires_origin)
	{
		auto const dir_orbit = sun_loc - (earth_loc + view_offset);

		hires_direction const n{view_offset};
		hires_direction const te{hires_displacement{-view_offset[1], view_offset[0], 0.0}};
		auto const ts = cross(te, n);

		geosimd::mat_4x4 const m{
			std::array<geosimd::vec_t<double, 4>, 4>{
				te.get().get(), ts.get().get(), n.get().get(), geosimd::vec_t{0.0, 0.0, 0.0, 1.0}
			}
		};

		return hires_direction{hires_displacement{transposed(m)*dir_orbit.get().get()}};
	}

}

#endif