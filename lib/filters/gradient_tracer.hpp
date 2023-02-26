#ifndef TERRAFORMER_LIB_IMAGE_GRAD_TRACER_HPP
#define TERRAFORMER_LIB_IMAGE_GRAD_TRACER_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/array_tuple.hpp"

namespace terraformer
{
	auto trace_gradient(span_2d<float const> pixels, pixel_coordinates start_point)
	{
		array_tuple<location, float> ret;

		location loc
		{
			static_cast<float>(start_point.x),
			static_cast<float>(start_point.y),
			0.0f
		};

		auto z = pixels(start_point.x, start_point.y);

		while(true)
		{
			auto const g = grad(pixels, loc[0], loc[1], 1.0f);
			auto const grad_size = norm(g);
			if(grad_size < 1.0f/(1024.0f*1024.0f))
			{ return ret; }

			auto const loc_next = loc - g/grad_size;
			if(!inside(pixels, loc_next[0], loc_next[1]))
			{ return ret; }

			auto const z_next = interp(pixels, loc_next[0], loc_next[1]);
			if(z_next >= z)
			{ return ret; }

			loc = loc_next;
			z = z_next;
			ret.push_back(tuple{location{loc[0], loc[1], z}, grad_size});
		}

		return ret;
	}

	auto trace_gradient_periodic_xy(span_2d<float const> pixels, pixel_coordinates start_point)
	{
		array_tuple<location, float> ret;

		location loc
		{
			static_cast<float>(start_point.x),
			static_cast<float>(start_point.y),
			0.0f
		};

		auto z = pixels(start_point.x, start_point.y);

		while(true)
		{
			auto const g = grad(pixels, loc[0], loc[1], 1.0f);
			auto const grad_size = norm(g);
			if(grad_size < 1.0f/(1024.0f*1024.0f))
			{ return ret; }

			auto const loc_next = loc - g/grad_size;
			if(loc_next[1] >= static_cast<float>(pixels.height()) || loc_next[1] < 0.0f)
			{ return ret; }

			auto const z_next = interp(pixels, loc_next[0], loc_next[1]);
			if(z_next >= z)
			{ return ret; }

			loc = loc_next;
			z = z_next;
			ret.push_back(tuple{location{loc[0], loc[1], z}, grad_size});
		}

		return ret;
	}
}

#endif