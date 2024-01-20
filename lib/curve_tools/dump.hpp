#ifndef TERRAFORMER_CURVE_DUMP_HPP
#define TERRAFORMER_CURVE_DUMP_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/string_converter.hpp"

#include <span>
#include <cstdio>

namespace terraformer
{
	template<size_t Index>
	inline void write_values(FILE* output, std::span<location const> vals)
	{
		if(std::size(vals) == 0)
		{ return; }

		auto str = to_string_helper(get<Index>(vals[0]));
		fprintf(output, "%s", str.c_str());
		for(size_t k = 1; k != std::size(vals); ++k)
		{
			str = to_string_helper(get<Index>(vals[k]));
			fprintf(output, ",\n%s", str.c_str());
		}
	}

	inline void curve_dump(FILE* output, std::span<location const> vals)
	{
		fprintf(output, R"({
	"data_points": {
		"x": [
)");
		write_values<0>(output, vals);
		fprintf(output, R"(],
		"y": [
)");
		write_values<1>(output, vals);
		fprintf(output, R"(]
	}
})");
	}

	inline void write_values(FILE* output, std::span<std::vector<location> const> vals)
	{
		if(std::size(vals) == 0)
		{ return; }

		curve_dump(output, vals[0]);
		for(size_t k = 1; k != std::size(vals); ++k)
		{
			fprintf(output, ",\n");
			curve_dump(output, vals[k]);
		}
	}

	class curve_set
	{
	public:
		curve_set& append(std::span<location const> curve_to_store)
		{
			m_curves.push_back(curve{std::begin(curve_to_store), std::end(curve_to_store)});
			return *this;
		}

		void write_to(FILE* output) const
		{
			fprintf(output, R"({
	"curves": [
)");
			write_values(output, m_curves);
		fprintf(output, R"(],
	"axes_config": {
		"ratio":"scaled",
		"x":{
			"grid_lines":{}
		},
		"y":{
			"grid_lines":{}
		}
	}
})");
		}

	private:
		using curve = std::vector<location>;
		std::vector<curve> m_curves;
	};
}

#endif
