//@	{"target":{"name":"main.o"}}

#include "./mathutils.hpp"

#include <cassert>

struct Foo
{
	int x;
	int y;
};


int main()
{
	Vector a{3, 4, 0};
	static_assert(std::is_same_v<decltype(length(a)), double>);
	assert(length(a) == 5);
}