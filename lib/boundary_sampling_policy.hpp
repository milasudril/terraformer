#ifndef TERRAFORMER_BOUNDARY_SAMPLING_POLICY_HPP
#define TERRAFORMER_BOUNDARY_SAMPLING_POLICY_HPP

#include <cstdint>
#include <concepts>

namespace terraformer
{
	template<class T>
	concept boundary_sampling_policy
		= requires(T const& policy, float fx, int32_t ix, uint32_t ux, uint32_t max_size)
	{
		{policy(fx, max_size)} -> std::same_as<float>;
		{policy(ix, max_size)} -> std::same_as<uint32_t>;
		{policy(ux, max_size)} -> std::same_as<uint32_t>;
	};
}

#endif