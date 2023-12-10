//@	{"dependencies_extra":[{"ref":"./find_zeros.o", "rel":"implementation"}]}

#include <span>
#include <vector>

namespace terraformer
{
	std::vector<size_t> find_zeros(std::span<float const>, double margin_factor = 0.5);
}