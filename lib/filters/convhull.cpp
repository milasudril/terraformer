//@	{"target":{"name":"convhull.o"}}

#include "./convhull.hpp"

#include "lib/curve_tool/polynomial.hpp"

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

	auto const max = std::ranges::max_element(values, [](auto const a, auto const b){
		return a.y < b.y;
	});

	auto const left = values.front();
	auto const right = values.back();
	auto const split_at = max;

	buffer.push_back(*max);
	auto i = std::begin(values);
	{
		auto const size_part = split_at - std::begin(values);
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
		auto const size_part = std::end(values) - split_at;
		std::vector<loc_2d> part{};
		part.reserve(size_part);
		auto const dydx = (right.y - max->y)/(right.x - max->x);
		terraformer::polynomial const p{max->y, dydx};
		while(i != std::end(values))
		{
			if(i->y > p(i->x))
			{ part.push_back(*i); }

			++i;
		}
		convhull(part, buffer);
	}
}
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
