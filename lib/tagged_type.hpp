#ifndef TERRAFORMER_TAGGEDTYPE_HPP
#define TERRAFORMER_TAGGEDTYPE_HPP

template<class T, auto tag>
class TaggedType
{
public:
	constexpr explicit TaggedType(T&& value):m_value{value}{}

	constexpr operator T() const
	{
		return m_value;
	}

private:
	T m_value;
};

#endif