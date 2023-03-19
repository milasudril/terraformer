//@	{"target":{"name":"convhull.o"}}

#include "./convhull.hpp"

#include "lib/curve_tool/polynomial.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/mesh_store/mesh.hpp"
#include "lib/pixel_store/image.hpp"

#include <algorithm>

namespace
{
	struct loc_2d
	{
		float x;
		float y;
	};

	bool is_ccw(loc_2d prevprev, loc_2d prev, loc_2d current)
	{
		auto const dx_1 = prev.x - prevprev.x;
		auto const dy_1 = prev.y - prevprev.y;
		auto const dx_2 = current.x - prev.x;
		auto const dy_2 = current.y - prev.y;

		return dy_2/dx_2 > dy_1/dx_1;
	}


	void convhull(std::span<loc_2d const> values, std::vector<loc_2d>& buffer)
	{
		if(std::size(values) <= 2)
		{
			for(size_t k = 0; k != std::size(values); ++k)
			{
				buffer.push_back(values[k]);
			}
			return;
		}

		auto const max = std::max_element(std::begin(values) + 1,
			std::end(values) - 1,
			[](auto const a, auto const b){
				return a.y < b.y;
			});

		auto const left = values.front();
		auto const right = values.back();
		auto const split_at = max;

		buffer.push_back(*max);
		auto i = std::begin(values) + 1;
		{
			auto const size_part = split_at - (std::begin(values) + 1);
			std::vector<loc_2d> part{};
			part.reserve(size_part);
			auto const dydx = (max->y - left.y)/(max->x - left.x);
			terraformer::polynomial const p{left.y, dydx};
			while(i != max)
			{
				if(i->y > p(i->x))
				{ part.push_back(*i); }

				++i;
			}
			convhull(part, buffer);
		}

		{
			++i;
			auto const size_part = std::end(values) - (split_at + 2);
			std::vector<loc_2d> part{};
			part.reserve(size_part);
			auto const dydx = (right.y - max->y)/(right.x - max->x);
			terraformer::polynomial const p{max->y, dydx};
			while(i != std::end(values) - 1)
			{
				if(i->y > p(i->x - max->x))
				{ part.push_back(*i); }

				++i;
			}

			convhull(part, buffer);
		}
	}

	void convhull(terraformer::span_2d<float const> image,
		terraformer::span_2d<bool> visited,
		terraformer::mesh& mesh_to_build,
		terraformer::face const& initial_face,
		terraformer::location split_at);

	void append_new_loc_if_above(terraformer::location loc,
		terraformer::span_2d<float const> image,
		terraformer::span_2d<bool> visited,
		std::vector<terraformer::location>& new_locations)
	{
		auto const loc_x = static_cast<uint32_t>(loc[0]);
		auto const loc_y = static_cast<uint32_t>(loc[1]);

		if(!visited(loc_x, loc_y) && image(loc_x, loc_y) > loc[2])
		{
			visited(loc_x, loc_y) = true;
			new_locations.push_back(loc);
		}
	}

	void visit(terraformer::span_2d<float const> image,
		terraformer::span_2d<bool> visited,
		terraformer::mesh& mesh_to_build,
		terraformer::face const& initial_face)
	{
		auto by_z = [](terraformer::location a, terraformer::location b){
			return a[2] < b[2];
		};

		std::vector<terraformer::location> new_locations;
		auto const t = resolve(initial_face, mesh_to_build.locations());
		project_from_above(t, append_new_loc_if_above, image, visited, new_locations);

		auto const max = std::ranges::max_element(new_locations, by_z);
		convhull(image, visited, mesh_to_build, initial_face, *max);
	}

	void convhull(terraformer::span_2d<float const> image,
		terraformer::span_2d<bool> visited,
		terraformer::mesh& mesh_to_build,
		terraformer::face const& initial_face,
		terraformer::location split_at)
	{
		auto const new_triangles = mesh_to_build.subdivide(initial_face, split_at);
		for(size_t k = 0; k != std::size(new_triangles); ++k)
		{ visit(image, visited, mesh_to_build, initial_face); }
	}
}

std::vector<float> terraformer::convhull2(std::span<float const> values)
{
	std::vector<loc_2d> temp;
	temp.reserve(std::size(values));
	for(size_t k = 0; k != std::size(values); ++k)
	{
		auto const x = static_cast<float>(k);
		temp.push_back(loc_2d{x, values[k]});
	}

	std::vector<loc_2d> output{temp.front(), temp.back()};
	convhull(temp, output);
	std::ranges::sort(output, [](auto const a, auto const b){
		return a.x < b.x;
	});

#if 0
	for(size_t k = 0; k != std::size(output);++k)
	{
		printf("%.8g %.8g\n", output[k].x, output[k].y);
	}
	printf("==============\n");
#endif

	std::vector<float> ret;
	ret.reserve(std::size(values));
	for(size_t k = 1; k != std::size(output); ++k)
	{
		auto const dx = output[k].x - output[k - 1].x;
		auto const dy = output[k].y - output[k - 1].y;

		polynomial const p{output[k - 1].y, dy/dx};
	//	printf("dy/dx = %.8g\n", dy/dx);
		for(size_t l = 0; l != static_cast<size_t>(dx); ++l)
		{
			auto const x = static_cast<float>(l);
		//	printf("%.8g %.8g\n", x, p(x));
			ret.push_back(p(x));
		}
	//	printf("-----------------\n");
	}
	ret.push_back(output.back().y);
//	printf("==============\n");
	return ret;
}

terraformer::basic_image<float> terraformer::convhull2(span_2d<float const> input)
{
	basic_image<float> ret{input.width(), input.height()};
	basic_image<bool> visited{input.width(), input.height()};
	terraformer::mesh mesh_to_build;
	auto const x_max = input.width() - 1;
	auto const y_max = input.height() - 1;
	auto const x_maxf = static_cast<float>(input.width() - 1);
	auto const y_maxf = static_cast<float>(input.height() - 1);
	direction const n{displacement{0.0f, 0.0f, 1.0f}};

	mesh_to_build.push_back(tuple{location{0.0f, 0.0f, input(0u, 0u)}, n});
	mesh_to_build.push_back(tuple{location{x_maxf, 0.0f, input(x_max, 0u)}, n});
	mesh_to_build.push_back(tuple{location{x_maxf, y_maxf, input(x_max, y_max)}, n});
	mesh_to_build.push_back(tuple{location{0.0f, y_maxf, input(0u, y_max)}, n});

	face const f1{0u, 1u, 2u};
	face const f2{2u, 3u, 1u};
	mesh_to_build.insert(f1);
	mesh_to_build.insert(f2);

	visit(input, visited, mesh_to_build, f1);
	visit(input, visited, mesh_to_build, f2);

/*

	{
		std::vector<location> new_locations;
		auto const t = resolve(new_triangles[k], mesh_to_build.locations());
		project_from_above(t, append_new_loc_if_above, image, visited, new_locations);

		auto const max = std::ranges::max_element(new_locations, by_z);
		convhull(image, visited, mesh_to_build, new_triangles[k], *max);
	}

*/
	return ret;
}

void terraformer::convhull(std::span<float> values)
{
	std::vector<loc_2d> buffer;
	for(size_t k = 0; k != std::size(values); ++k)
	{
		loc_2d const val{static_cast<float>(k), values[k]};

		while(std::size(buffer) >= 2)
		{
			auto const prevprev = *(std::end(buffer) - 2);
			auto const prev = *(std::end(buffer) - 1);
			if(!is_ccw(prevprev, prev, val))
			{ break; }

			buffer.pop_back();
		}

		buffer.push_back(val);
	}

	std::vector<float> ret(std::size(values));
	auto loc_prev = buffer[0];
	for(size_t k = 1; k != std::size(buffer); ++k)
	{
		auto const loc_current = buffer[k];
		auto const dx = loc_current.x - loc_prev.x;
		auto const dy = loc_current.y - loc_prev.y;
		auto const slope = dy/dx;
		for(size_t l = 0; l != static_cast<size_t>(dx); ++l)
		{
			auto const y = loc_prev.y + static_cast<float>(l)*slope;
			values[static_cast<size_t>(loc_prev.x) + l] = y;
		}
		loc_prev = loc_current;
	}
}



void terraformer::convhull_per_scanline(span_2d<float> buffer)
{
	for(uint32_t k = 0; k != buffer.height(); ++k)
	{
		convhull(buffer.row(k));
	}
}

void terraformer::convhull_per_column(span_2d<float> buffer)
{
	std::vector<float> colbuff(buffer.height());
	for(uint32_t x = 0; x != buffer.width(); ++x)
	{
		for(uint32_t y = 0; y != buffer.height(); ++y)
		{ colbuff[y] = buffer(x, y); }

		convhull(colbuff);

		for(uint32_t y = 0; y!= buffer.height(); ++y)
		{ buffer(x, y) = std::max(buffer(x, y), colbuff[y]); }
	}
}
