//@	{
//@	 "target":{ "name":"font_mapper.o" ,"pkgconfig_libs":["fontconfig"]}
//@	}

#include "./font_mapper.hpp"

terraformer::ui::font_handling::font_mapper::font_mapper(): m_handle{FcInitLoadConfigAndFonts()}
{
	if(m_handle == nullptr)
	{ throw std::runtime_error{"Failed to initialize fc"}; }
}

namespace
{
	struct FcPatternDeleter
	{
		void operator()(FcPattern* pattern)
		{
			FcPatternDestroy(pattern);
		}
	};
}

std::filesystem::path terraformer::ui::font_handling::font_mapper::get_path(char const* font) const
{
	auto pattern = std::unique_ptr<FcPattern, FcPatternDeleter>{
	   FcNameParse(reinterpret_cast<FcChar8 const*>(font))};
	if(pattern == nullptr)
	{ throw std::runtime_error{"Failed to parse font name"}; }

	auto const handle = m_handle.get();

	FcConfigSubstitute(handle, pattern.get(), FcMatchPattern);
	FcDefaultSubstitute(pattern.get());

	FcResult res;
	auto font_resource =
	   std::unique_ptr<FcPattern, FcPatternDeleter>{FcFontMatch(handle, pattern.get(), &res)};
	if(font_resource == nullptr)
	{ throw std::runtime_error{"Failed to create a font name matching object"}; }

	FcChar8* file;
	if(FcPatternGetString(font_resource.get(), FC_FILE, 0, &file) != FcResultMatch)
	{ throw std::runtime_error{"Failed to find a matching font"}; }

	return std::filesystem::path{reinterpret_cast<char const*>(file)};
}