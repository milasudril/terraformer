#ifndef TERRAFORMER_MATHUTILS_RATIO_HPP
#define TERRAFORMER_MATHUTILS_RATIO_HPP

#include "./polynomial.hpp"

#include <algorithm>

namespace terraformer
{
	class ratio
	{
	public:
		explicit ratio(float value): m_value{value}{}

		operator float() const
		{ return m_value; }

	private:
		float m_value;
	};
}

#endif