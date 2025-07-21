//@	{"target":{"name":"resource_reference.test"}}

#include "./resource_reference.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct dummy_vtable
	{
		template<class T>
		constexpr dummy_vtable(T){}
	};

	struct my_vtable
	{
		template<class ReferencedType, class Implementation>
		constexpr explicit my_vtable(
			std::type_identity<ReferencedType>,
			std::type_identity<Implementation>
		):
		do_stuff{
			[](void* obj) {
				Implementation::do_stuff(*static_cast<ReferencedType*>(obj));
			}
		}
		{}

		void (*do_stuff)(void* obj);
	};
};


TESTCASE(terraformer_resource_reference_default_state_is_falsy)
{
	terraformer::resource_reference<dummy_vtable> res;
	EXPECT_EQ(res, false);
}

TESTCASE(terraformer_resource_reference_decoupled_vtable)
{
	int x = 0;
	struct do_stuff_impl
	{
		static void do_stuff(int& value)
		{ value = 1; }
	};

	terraformer::resource_reference<my_vtable> resource_ref{std::ref(x), std::type_identity<do_stuff_impl>{}};

	EXPECT_EQ(x, 0);

	auto const vt = resource_ref.get_vtable();
	EXPECT_EQ(x, 0);

	auto const pointer = resource_ref.get_pointer();
	EXPECT_EQ(x, 0);

	EXPECT_EQ(pointer, &x);
	vt.do_stuff(pointer);
	EXPECT_EQ(x, 1);
}