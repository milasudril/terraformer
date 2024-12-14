//@	{"target":{"name":"gl_resource_factory.test"}}

#include "./gl_resource_factory.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_drawing_api_gl_resource_factory_create_texture)
{
	glCreateTextures = [](GLenum target, GLsizei n, GLuint* textures){
		EXPECT_EQ(target, GL_TEXTURE_2D);
		EXPECT_EQ(n, 1);
		REQUIRE_NE(textures, nullptr);
		textures[0] = 123;
	};
	glTextureStorage2D = [](GLuint texture, GLsizei, GLenum, GLsizei, GLsizei){
		EXPECT_EQ(texture, 123);
	};
	glTextureParameteri = [](GLuint texture, GLenum, GLint){
		EXPECT_EQ(texture, 123);
	};
	glTextureSubImage2D = [](GLuint texture, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void const*){
		EXPECT_EQ(texture, 123);
	};
	glGenerateTextureMipmap = [](GLuint texture){
		EXPECT_EQ(texture, 123);
	};

	terraformer::image img{320, 220};
	auto res = terraformer::ui::drawing_api::gl_resource_factory::create(
		std::type_identity<terraformer::ui::main::texture>{},
		555,
		img
	);

	EXPECT_EQ(res.get(), true);
	EXPECT_EQ(res.belongs_to_backend(555), true);
}