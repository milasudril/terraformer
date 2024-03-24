//@	{
//@	"dependencies_extra":[{"ref":"./font_mapper.o","rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_UI_FONT_HANDLING_FONT_MAPPER_HPP
#define TERRAFORMER_UI_FONT_HANDLING_FONT_MAPPER_HPP

#include <fontconfig/fontconfig.h>

#include <filesystem>
#include <memory>

namespace terraformer::ui::font_handling
{
	struct fc_config_deleter
	{
		void operator()(FcConfig* handle)
		{
			if(handle != nullptr)
			{ FcConfigDestroy(handle); }
		}
	};

	class font_mapper
	{
	public:
		font_mapper();

		std::filesystem::path get_path(char const* font) const;

		bool valid() const
		{ return m_handle != nullptr; }

	private:
		std::unique_ptr<FcConfig, fc_config_deleter> m_handle;
	};
}

#endif