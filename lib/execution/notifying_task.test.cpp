//@	{"target":{"name":"notifying_task.test"}}

#include "./notifying_task.hpp"

#include "testfwk/testfwk.hpp"

namespace
{
	enum class init_state:int{init, copied, moved, copy_assigned, move_assigned};

	char const* to_string(init_state val)
	{
		switch(val)
		{
			case init_state::init:
				return "init";
			case init_state::copied:
				return "copied";
			case init_state::moved:
				return "moved";
			case init_state::copy_assigned:
				return "copy_assigned";
			case init_state::move_assigned:
				return "move_assigned";
		}
		__builtin_unreachable();
	}

	struct special_member_functions
	{
		special_member_functions(special_member_functions const&):state{init_state::copied}
		{}

		special_member_functions(special_member_functions&&):state{init_state::moved}
		{}

		special_member_functions& operator=(special_member_functions const&)
		{
			state = init_state::copy_assigned;
			return *this;
		}

		special_member_functions& operator=(special_member_functions&&)
		{
			state = init_state::move_assigned;
			return *this;
		}

		special_member_functions() = default;

		init_state state;
	};

	struct retval
	{
		explicit retval(int x):value{x}{}

		int value;
		special_member_functions memfun;
	};

	struct event
	{
		bool status{};
	};

	void trigger(event& event)
	{
		event.status = true;
	}
}

TESTCASE(notifying_task_call_with_non_movable_object)
{
	event e;
	auto const n = 2;
	terraformer::notifying_task func{std::ref(e), [](auto arg, int val){
		EXPECT_EQ(arg, 1);
		EXPECT_EQ(val, 2);
		return retval{arg};
	}, 1, n};

	EXPECT_EQ(e.status, false);
	auto val = func();
	EXPECT_EQ(e.status, true);
	EXPECT_EQ(val.value, 1);
	EXPECT_NE(val.memfun.state, init_state::copy_assigned);
	EXPECT_NE(val.memfun.state, init_state::copied);
}