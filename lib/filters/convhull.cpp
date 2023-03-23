//@	{"target":{"name":"convhull.o"}}

#include "./convhull.hpp"

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
}

std::vector<float> terraformer::convhull(std::span<float const> values)
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
			ret[static_cast<size_t>(loc_prev.x) + l] = y;
		}
		loc_prev = loc_current;
	}
	ret.back() = values.back();
	return ret;
}

terraformer::basic_image<float> terraformer::convhull_per_scanline(span_2d<float const> buffer)
{
	basic_image<float> ret{buffer.width(), buffer.height()};
	for(uint32_t k = 0; k != buffer.height(); ++k)
	{
		ret.pixels().update_row(k, convhull(buffer.row(k)));
	}
	return ret;
}

terraformer::basic_image<float> terraformer::convhull_per_column(span_2d<float const> buffer)
{
	basic_image<float> ret{buffer.width(), buffer.height()};
	std::vector<float> colbuff(buffer.height());
	for(uint32_t x = 0; x != buffer.width(); ++x)
	{
		for(uint32_t y = 0; y != buffer.height(); ++y)
		{ colbuff[y] = buffer(x, y); }

		auto temp = convhull(colbuff);

		for(uint32_t y = 0; y!= buffer.height(); ++y)
		{ ret(x, y) = temp[y]; }
	}
	return ret;
}
