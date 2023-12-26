//@	{"dependencies_extra":[{"ref":"./find_zeros.o", "rel":"implementation"}]}

#include <span>
#include <vector>

namespace terraformer
{
	struct find_zeros_result
	{
		std::vector<size_t> zeros;
		float first_value;
	};

	find_zeros_result find_zeros(std::span<float const>, float margin_factor = 0.3f);
}
