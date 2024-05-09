//@	{"target":{"name":"glyph_renderer.o"}}

#include "./glyph_renderer.hpp"

char const* terraformer::ui::font_handling::get_ft_error_message(FT_Error err)
{
	#undef FTERRORS_H_
	#define FT_ERRORDEF(error_code, value, string) case error_code: return string;
	#define FT_ERROR_START_LIST switch(err) {
	#define FT_ERROR_END_LIST default: return "Unknown error"; }
	#include FT_ERRORS_H
}