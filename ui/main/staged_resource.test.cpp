//@	{"target":{"name":"staged_resource.test"}}

#include "./staged_resource.hpp"

#include <testfwk/testfwk.hpp>
#include <utility>

namespace
{
	struct backend_resource
	{
		bool belongs_to_backend(uint64_t object_id) const
		{ return backend_id == object_id; }

		int value{-1};
		uint64_t backend_id{static_cast<uint64_t>(-1)};
		size_t upload_callcount{0};

		void upload(int new_value)
		{
			++upload_callcount;
			value = new_value;
		}
	};

	struct backend_resource_backend:terraformer::global_instance_counter
	{
		size_t num_objects_created{0};

		backend_resource_backend() = default;

		backend_resource_backend(backend_resource_backend const& other):
			terraformer::global_instance_counter{other},
			num_objects_created{0}
		{}

		backend_resource create(std::type_identity<backend_resource>, int value)
		{
			++num_objects_created;
			return backend_resource{
				.value = value,
				.backend_id = get_global_id()
			};
		}
	};
}

TESTCASE(terraformer_ui_main_staged_resource_first_call_to_get_backend_resource_creates_new_resource)
{
	terraformer::ui::main::staged_resource<backend_resource, int> resource{123};
	backend_resource_backend the_backend_resource_backend;
	auto& the_backend_resource = resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.value, 123);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 0);
}

namespace
{
	template<class T>
	bool returns_ref_to_const(T const&)
	{ return true; }

	template<class T>
	bool returns_ref_to_const(T&)
	{ return false; }
}

TESTCASE(terraformer_ui_main_staged_resource_cast_to_frontend_resource_is_ref_to_const)
{
	terraformer::ui::main::staged_resource<backend_resource, int> resource{123};
	EXPECT_EQ(returns_ref_to_const<int>(resource), true);
	EXPECT_EQ(returns_ref_to_const<int>(std::as_const(resource)), true);
}

TESTCASE(terraformer_ui_main_staged_resource_different_backend_recreates_backend_resource)
{
	terraformer::ui::main::staged_resource<backend_resource, int> resource{123};
	backend_resource_backend the_backend_resource_backend;
	auto& the_backend_resource = resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.value, 123);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 0);

	auto other_backend = the_backend_resource_backend;
	EXPECT_EQ(other_backend.num_objects_created, 0);

	resource.get_backend_resource(other_backend);
	EXPECT_NE(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource.backend_id, other_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(other_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 0);
}

TESTCASE(terraformer_ui_main_staged_resource_assigning_frontend_resource_triggers_upload)
{
	terraformer::ui::main::staged_resource<backend_resource, int> resource{123};
	backend_resource_backend the_backend_resource_backend;
	auto& the_backend_resource = resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.value, 123);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 0);

	resource = 56;

	resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 1);
}

TESTCASE(terraformer_ui_main_staged_resource_fetch_initially_cached_value)
{
	terraformer::ui::main::staged_resource<backend_resource, int> resource{123};
	backend_resource_backend the_backend_resource_backend;
	auto& the_backend_resource = resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.value, 123);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 0);

	resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 0);
}

TESTCASE(terraformer_ui_main_staged_resource_fetch_cached_value_after_update)
{
	terraformer::ui::main::staged_resource<backend_resource, int> resource{123};
	backend_resource_backend the_backend_resource_backend;
	auto& the_backend_resource = resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.value, 123);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 0);

	resource = 56;

	resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 1);

	resource.get_backend_resource(the_backend_resource_backend);
	EXPECT_EQ(the_backend_resource.backend_id, the_backend_resource_backend.get_global_id());
	EXPECT_EQ(the_backend_resource_backend.num_objects_created, 1);
	EXPECT_EQ(the_backend_resource.upload_callcount, 1);
}