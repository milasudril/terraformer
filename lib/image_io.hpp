//@	{
//@	"dependencies_extra":[{"ref":"./image_io.o", "rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_IMAGEIO_HPP
#define TERRAFORMER_IMAGEIO_HPP

#include "./image.hpp"
#include "./ilm_io.hpp"

#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfInputFile.h>

template<class T>
struct Empty
{
	using type = T;
};

namespace detail
{
	using OutputFileFactory = Imf::OutputFile (*)(void*, Imf::Header const&);
	using InputFileFactory  = Imf::InputFile (*)(void*);
}

Image load(Empty<Image>, void* arg, detail::InputFileFactory make_input_file);

inline Image load(Empty<Image>, char const* filename)
{
	return load(Empty<Image>{}, const_cast<char*>(filename), [](void* filename) {
		return Imf::InputFile{static_cast<char const*>(filename)};
	});
}

template<class FileReader>
Image load(Empty<Image>, FileReader&& reader)
{
	IlmInputAdapter input{std::forward<FileReader>(reader)};
	return load(Empty<Image>{}, &input, [](void* input) {
		return Imf::InputFile{*static_cast<IlmInputAdapter<FileReader>*>(input)};
	});
}

bool fileValid(Empty<Image>, char const* filename);


void store(Span2d<RgbaPixel const> pixels, void* arg, detail::OutputFileFactory make_output_file);

inline void store(Span2d<RgbaPixel const> pixels, char const* filename)
{
	store(pixels, const_cast<char*>(filename), [](void* filename, Imf::Header const& header) {
		return Imf::OutputFile{static_cast<char const*>(filename), header};
	});
}

inline void store(Image const& img, char const* filename) { store(img.pixels(), filename); }

template<class FileWriter>
void store(Span2d<RgbaPixel const> pixels, FileWriter&& writer)
{
	IlmOutputAdapter output{std::forward<FileWriter>(writer)};
	store(pixels, &output, [](void* output, Imf::Header const& header) {
		return Imf::OutputFile{*static_cast<IlmOutputAdapter<FileWriter>*>(output), header};
	});
}

template<class FileWriter>
void store(Image const& img, FileWriter&& writer)
{
	store(img.pixels(), std::forward<FileWriter>(writer));
}

#endif