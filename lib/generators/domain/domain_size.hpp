#ifndef TERRAFORMER_DOMAIN_SIZE_HPP
#define TERRAFORMER_DOMAIN_SIZE_HPP

namespace terraformer
{
	struct domain_size_descriptor
	{
		float width = 4096.0f;
		float height = 4096.0f;

		constexpr bool operator==(domain_size_descriptor const&) const = default;
		constexpr bool operator!=(domain_size_descriptor const&) const = default;
	};
}

#endif