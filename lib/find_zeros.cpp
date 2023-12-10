//@	{"target":{"name":"find_zeros.o"}}

#include "./find_zeros.hpp"

std::vector<size_t> terraformer::find_zeros(std::span<float const> data_points, double margin_factor)
{
	std::vector<size_t> intercept_index;
	auto positive = data_points[0] >= 0.0f;

	if(data_points[0] == 0.0f)
	{ intercept_index.push_back(0); }

	for(size_t k = 1; k != std::size(data_points); ++k)
	{
		if(positive && data_points[k] < 0.0f)
		{
			intercept_index.push_back(k);
			positive = false;
		}
		if(!positive && data_points[k] >= 0.0f)
		{
			intercept_index.push_back(k);
			positive = true;
		}
	}

	auto const avg_distnace = static_cast<double>(std::size(data_points))
		/static_cast<double>(std::size(intercept_index));
	std::vector<size_t> intercept_index_filtered{intercept_index[0]};
	size_t k_start = 0;
	auto k_start_old = k_start;
	auto skip = false;
	for(size_t k = 1; k != std::size(intercept_index); ++k)
	{
		auto const d = static_cast<double>(intercept_index[k] - intercept_index[k_start]);
		if(!skip)
		{
			if(d > margin_factor*avg_distnace)
			{
				intercept_index_filtered.push_back(intercept_index[k]);
				k_start_old = k_start;
				k_start = k;
			}
			else
			{
				intercept_index_filtered.pop_back();
				k_start = k_start_old;
				skip = true;
			}
		}
		else
		{
			if(d > margin_factor*avg_distnace)
			{
				intercept_index_filtered.push_back(intercept_index[k]);
				k_start_old = k_start;
				k_start = k;
				skip = false;
			}
		}
	}
	return intercept_index_filtered;
}