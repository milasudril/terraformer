//@	{
//@	"dependencies_extra":[{"ref":"./image_io.o", "rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_IMAGEIO_HPP
#define TERRAFORMER_IMAGEIO_HPP

#include "./image.hpp"
#include "./ilm_io.hpp"
#include "lib/common/utils.hpp"

#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfInputFile.h>

namespace terraformer
{

	namespace image_io_detail
	{
		using output_file_factory = Imf::OutputFile (*)(void*, Imf::Header const&);
		using input_file_factory  = Imf::InputFile (*)(void*);
	}

	// FIXME: This file needs FileReader and FileWriter concepts

	image load(empty<image>,
		void* arg,
		image_io_detail::input_file_factory make_input_file);

	inline image load(empty<image>, char const* filename)
	{
		return load(empty<image>{}, const_cast<char*>(filename), [](void* filename) {
			return Imf::InputFile{static_cast<char const*>(filename)};
		});
	}

	template<class FileReader>
	requires(!std::is_same_v<std::decay_t<FileReader>, char*>)
	image load(empty<image>, FileReader&& reader)
	{
		ilm_input_adapter input{std::forward<FileReader>(reader)};
		return load(empty<image>{}, &input, [](void* input) {
			return Imf::InputFile{*static_cast<ilm_input_adapter<FileReader>*>(input)};
		});
	}

	void store(span_2d<rgba_pixel const> pixels,
		void* arg, image_io_detail::output_file_factory make_output_file);

	inline void store(span_2d<rgba_pixel const> pixels, char const* filename)
	{
		store(pixels, const_cast<char*>(filename), [](void* filename, Imf::Header const& header) {
			return Imf::OutputFile{static_cast<char const*>(filename), header};
		});
	}

	inline void store(image const& img, char const* filename) { store(img.pixels(), filename); }

	template<class FileWriter>
	requires(!std::is_same_v<std::decay_t<FileWriter>, char*>)
	void store(span_2d<rgba_pixel const> pixels, FileWriter&& writer)
	{
		ilm_output_adapter output{std::forward<FileWriter>(writer)};
		store(pixels, &output, [](void* output, Imf::Header const& header) {
			return Imf::OutputFile{*static_cast<ilm_output_adapter<FileWriter>*>(output), header};
		});
	}

	template<class FileWriter>
	requires(!std::is_same_v<std::decay_t<FileWriter>, char*>)
	void store(image const& img, FileWriter&& writer)
	{
		store(img.pixels(), std::forward<FileWriter>(writer));
	}



	grayscale_image load(empty<grayscale_image>,
		void* arg,
		image_io_detail::input_file_factory make_input_file);

	inline grayscale_image load(empty<grayscale_image>, char const* filename)
	{
		return load(empty<grayscale_image>{}, const_cast<char*>(filename), [](void* filename) {
			return Imf::InputFile{static_cast<char const*>(filename)};
		});
	}

	template<class FileReader>
	requires(!std::is_same_v<std::decay_t<FileReader>, char*>)
	grayscale_image load(empty<grayscale_image>, FileReader&& reader)
	{
		ilm_input_adapter input{std::forward<FileReader>(reader)};
		return load(empty<grayscale_image>{}, &input, [](void* input) {
			return Imf::InputFile{*static_cast<ilm_input_adapter<FileReader>*>(input)};
		});
	}

	void store(span_2d<float const> pixels,
		void* arg, image_io_detail::output_file_factory make_output_file);

	inline void store(span_2d<float const> pixels, char const* filename)
	{
		store(pixels, const_cast<char*>(filename), [](void* filename, Imf::Header const& header) {
			return Imf::OutputFile{static_cast<char const*>(filename), header};
		});
	}

	inline void store(grayscale_image const& img, char const* filename) { store(img.pixels(), filename); }

	template<class FileWriter>
	void store(span_2d<float const> pixels, FileWriter&& writer)
	{
		ilm_output_adapter output{std::forward<FileWriter>(writer)};
		store(pixels, &output, [](void* output, Imf::Header const& header) {
			return Imf::OutputFile{*static_cast<ilm_output_adapter<FileWriter>*>(output), header};
		});
	}

	template<class FileWriter>
	void store(grayscale_image const& img, FileWriter&& writer)
	{
		store(img.pixels(), std::forward<FileWriter>(writer));
	}
}

#endif