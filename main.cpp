//@	{"target":{"name":"main.o"}}

#include "./mathutils.hpp"

struct Foo
{
	int x;
	int y;
};


int main()
{
vec4_t<int> a{1, 2, 3, 4};
vec4_t<int> b{4, 3, 2, 1};
a += b;
}