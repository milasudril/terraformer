//@	{"target":{"name":"texture.test"}}

#include "./texture.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_main_texture_default_is_not_created)
{
	terraformer::ui::main::texture texture;
	EXPECT_EQ(texture.belongs_to_backend(0), false);
}

namespace
{
	struct dummy_texture
	{
		std::reference_wrapper<size_t> calls_to_bind;
		std::reference_wrapper<size_t> calls_to_upload;

		void bind(int)
		{
			++(calls_to_bind.get());
		}

		void upload(terraformer::span_2d<terraformer::rgba_pixel const>)
		{
			++(calls_to_upload.get());
		}
	};
}

TESTCASE(terraformer_ui_main_texture_create_backend_id_1_is_belongs_to_backend_1)
{
	size_t calls_to_bind{0};
	size_t calls_to_upload{0};
	terraformer::ui::main::texture texture{std::in_place_type_t<dummy_texture>{}, 1, calls_to_bind, calls_to_upload};

	EXPECT_EQ(texture.belongs_to_backend(0), false);
	EXPECT_EQ(texture.belongs_to_backend(1), true);
	EXPECT_EQ(texture.belongs_to_backend(2), false);

	EXPECT_EQ(calls_to_bind, 0);
	EXPECT_EQ(calls_to_upload, 0);
}

TESTCASE(terraformer_ui_main_texture_move_construct_preserves_belongs_to_backend)
{
	size_t calls_to_bind{0};
	size_t calls_to_upload{0};
	terraformer::ui::main::texture texture{std::in_place_type_t<dummy_texture>{}, 1, calls_to_bind, calls_to_upload};

	EXPECT_EQ(texture.belongs_to_backend(0), false);
	EXPECT_EQ(texture.belongs_to_backend(1), true);
	EXPECT_EQ(texture.belongs_to_backend(2), false);

	auto other = std::move(texture);
	EXPECT_EQ(other.belongs_to_backend(0), false);
	EXPECT_EQ(other.belongs_to_backend(1), true);
	EXPECT_EQ(other.belongs_to_backend(2), false);
	EXPECT_EQ(texture.belongs_to_backend(0), false);
	EXPECT_EQ(texture.belongs_to_backend(1), false);
	EXPECT_EQ(texture.belongs_to_backend(2), false);

	EXPECT_EQ(calls_to_bind, 0);
	EXPECT_EQ(calls_to_upload, 0);
}

TESTCASE(terraformer_ui_main_texture_move_construct_move_assign_replaces_belongs_to_backend)
{
	size_t calls_to_bind{0};
	size_t calls_to_upload{0};
	terraformer::ui::main::texture texture{std::in_place_type_t<dummy_texture>{}, 1, calls_to_bind, calls_to_upload};

	EXPECT_EQ(texture.belongs_to_backend(0), false);
	EXPECT_EQ(texture.belongs_to_backend(1), true);
	EXPECT_EQ(texture.belongs_to_backend(2), false);

	terraformer::ui::main::texture other{std::in_place_type_t<dummy_texture>{}, 2, calls_to_bind, calls_to_upload};
	EXPECT_EQ(other.belongs_to_backend(0), false);
	EXPECT_EQ(other.belongs_to_backend(1), false);
	EXPECT_EQ(other.belongs_to_backend(2), true);
	EXPECT_EQ(texture.belongs_to_backend(0), false);
	EXPECT_EQ(texture.belongs_to_backend(1), true);
	EXPECT_EQ(texture.belongs_to_backend(2), false);

	other = std::move(texture);
	EXPECT_EQ(other.belongs_to_backend(0), false);
	EXPECT_EQ(other.belongs_to_backend(1), true);
	EXPECT_EQ(other.belongs_to_backend(2), false);
	EXPECT_EQ(texture.belongs_to_backend(0), false);
	EXPECT_EQ(texture.belongs_to_backend(1), false);
	EXPECT_EQ(texture.belongs_to_backend(2), false);

	EXPECT_EQ(calls_to_bind, 0);
	EXPECT_EQ(calls_to_upload, 0);
}

TESTCASE(terraformer_ui_main_texture_upload)
{
	size_t calls_to_bind{0};
	size_t calls_to_upload{0};
	terraformer::ui::main::texture texture{std::in_place_type_t<dummy_texture>{}, 1, calls_to_bind, calls_to_upload};

	texture.upload(terraformer::span_2d<terraformer::rgba_pixel const>{});
	EXPECT_EQ(calls_to_bind, 0);
	EXPECT_EQ(calls_to_upload, 1);
}

TESTCASE(terraformer_ui_main_texture_bind)
{
	size_t calls_to_bind{0};
	size_t calls_to_upload{0};
	terraformer::ui::main::texture texture{std::in_place_type_t<dummy_texture>{}, 1, calls_to_bind, calls_to_upload};

	texture.bind(5);
	EXPECT_EQ(calls_to_bind, 1);
	EXPECT_EQ(calls_to_upload, 0);
}