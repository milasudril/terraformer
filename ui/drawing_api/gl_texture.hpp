#ifndef TERRAFORMER_UI_DRAWING_API_GL_TEXTURE_HPP
#define TERRAFORMER_UI_DRAWING_API_GL_TEXTURE_HPP

#include "./gl_resource.hpp"
#include "./gl_types.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer::ui::drawing_api
{
	struct gl_texture_deleter
	{
		void operator()(GLuint handle) const
		{
			glDeleteTextures(1, &handle);
		}
	};

	using gl_texture_handle = gl_resource<gl_texture_deleter>;

	struct gl_texture_descriptor
	{
		GLsizei width;
		GLsizei height;
		GLenum format;
		GLenum type;
		GLsizei num_mipmaps;

		auto operator<=>(gl_texture_descriptor const& other) const = default;
	};

	inline float aspect_ratio(gl_texture_descriptor const& descriptor)
	{
		return static_cast<float>(descriptor.width)/static_cast<float>(descriptor.height);
	}

	template<class T>
	struct to_gl_color_channel_layout;

	template<>
	struct to_gl_color_channel_layout<rgba_pixel>
	{
		static constexpr auto value = GL_RGBA;
	};

	inline GLenum gl_make_sized_format_red(GLenum type)
	{
		switch(type)
		{
			case GL_FLOAT:
				return GL_R32F;
			default:
				throw std::runtime_error{"Unimplemented type"};
		}
	}

	inline GLenum gl_make_sized_format_rg(GLenum type)
	{
		switch(type)
		{
			case GL_FLOAT:
				return GL_RG32F;
			default:
				throw std::runtime_error{"Unimplemented type"};
		}
	}

	inline GLenum gl_make_sized_format_rgb(GLenum type)
	{
		switch(type)
		{
			case GL_FLOAT:
				return GL_RGB32F;
			default:
				throw std::runtime_error{"Unimplemented type"};
		}
	}

	inline GLenum gl_make_sized_format_rgba(GLenum type)
	{
		switch(type)
		{
			case GL_FLOAT:
				return GL_RGBA32F;
			default:
				throw std::runtime_error{"Unimplemented type"};
		}
	}

	inline GLenum gl_make_sized_format(GLenum format, GLenum type)
	{
		switch(format)
		{
			case GL_RED:
				return gl_make_sized_format_red(type);
			case GL_RG:
				return gl_make_sized_format_rg(type);
			case GL_RGB:
				return gl_make_sized_format_rgb(type);
			case GL_RGBA:
				return gl_make_sized_format_rgba(type);
			default:
				throw std::runtime_error{"Unimplemented format"};
		}
	}

	inline size_t gl_get_sample_size(GLenum type)
	{
		switch(type)
		{
			case GL_FLOAT:
				return 4;
			default:
				throw std::runtime_error{"Unimplemented type"};
		}
	}

	inline size_t gl_get_pixel_size(GLenum format, GLenum type)
	{
		switch(format)
		{
			case GL_RED:
				return 1*gl_get_sample_size(type);
			case GL_RG:
				return 2*gl_get_sample_size(type);
			case GL_RGB:
				return 3*gl_get_sample_size(type);
			case GL_RGBA:
				return 4*gl_get_sample_size(type);
			default:
				throw std::runtime_error{"Unimplemented format"};
		}
	}

	inline auto get_image_size(gl_texture_descriptor const& descriptor)
	{
		return descriptor.width*descriptor.height*gl_get_pixel_size(descriptor.format, descriptor.type);
	}

	class gl_texture
	{
	public:
		explicit gl_texture():m_descriptor{0, 0, 0, 0, 0}{}

		explicit gl_texture(gl_texture_descriptor const& descriptor):gl_texture{}
		{
			set_format(descriptor);
		}

		void upload(std::span<std::byte const> data, gl_texture_descriptor const& descriptor)
		{
			if(descriptor != m_descriptor) [[unlikely]]
			{
				set_format(descriptor);
			}

			upload_impl(data);
		}

		template<class T>
		void upload(span_2d<T const> pixels, GLsizei num_mipmaps)
		{
			gl_texture_descriptor const descriptor{
				static_cast<GLsizei>(pixels.width()),
				static_cast<GLsizei>(pixels.height()),
				to_gl_color_channel_layout<T>::value,
				to_gl_type_id_v<T>,
				num_mipmaps
			};

			std::span const pixel_array{std::data(pixels), pixels.width()*pixels.height()};

			upload(std::as_bytes(pixel_array), descriptor);
		}

		void upload(std::span<std::byte const> data)
		{
			auto const image_size = get_image_size(m_descriptor);
			if(image_size != std::size(data)) [[unlikely]]
			{
				fprintf(stderr, "(!) Ignoring texture data of wrong size\n");
				return;
			}
			upload_impl(data);
		}

		void set_format(gl_texture_descriptor const& descriptor)
		{
			GLuint handle;
			glCreateTextures(GL_TEXTURE_2D, 1, &handle);
			glTextureStorage2D(handle,
				descriptor.num_mipmaps + 1,
				gl_make_sized_format(descriptor.format, descriptor.type),
				descriptor.width,
				descriptor.height);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, descriptor.num_mipmaps != 0 ?
				GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			m_handle.reset(handle);
			m_descriptor = descriptor;
		}

		void bind(GLenum texture_unit) const
		{
			glActiveTexture(texture_unit);
			glBindTexture(GL_TEXTURE_2D, m_handle.get());
		}

		auto const& descriptor() const
		{ return m_descriptor; }

	private:
		void upload_impl(std::span<std::byte const> data)
		{
			glTextureSubImage2D(m_handle.get(),
				0,  // mipmap level
				0,  // x-offset
				0,  // y-offset
				m_descriptor.width,
				m_descriptor.height,
				m_descriptor.format,
				m_descriptor.type,
				std::data(data));

			if(m_descriptor.num_mipmaps != 0) [[unlikely]]
			{ glGenerateTextureMipmap(m_handle.get()); }
		}

		gl_texture_handle m_handle;
		gl_texture_descriptor m_descriptor;
	};
}

#endif