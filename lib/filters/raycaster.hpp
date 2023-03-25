#ifndef TERRAFORMER_FILTERS_RAYCASTER_HPP
#define TERRAFORMER_FILTERS_RAYCASTER_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/spaces.hpp"

#include <optional>

namespace terraformer
{
	template<class Func>
	concept raycast_stop_predicate = requires(Func f, size_t k, location loc)
	{
		{f(k, loc)} -> std::same_as<bool>;
	};

	template<raycast_stop_predicate StopPredicate>
	inline std::optional<pixel_coordinates> raycast(span_2d<float const> heighmap,
		pixel_coordinates loc,
		float src_altitude,
		direction src_dir,
		float scale,
		StopPredicate&& pred)
	{
		auto const x_0 = static_cast<int32_t>(loc.x);
		auto const y_0 = static_cast<int32_t>(loc.y);
		location const r_0{static_cast<float>(x_0), static_cast<float>(y_0), src_altitude};

		size_t k = 0;
		auto r = r_0;
		displacement const v{src_dir[0], src_dir[1], scale*src_dir[2]};

		while(pred(k, r))
		{
			auto const z_ref = interp(heighmap, r[0], r[1]);
			if(k != 0 && r[2] < z_ref)
			{
				return pixel_coordinates{
					static_cast<uint32_t>(r[0] + 0.5f),
					static_cast<uint32_t>(r[1] + 0.5f)
				};
			}

			++k;
			r = r_0 + static_cast<float>(k)*v;
		}

		return std::nullopt;
	}
}

#endif