//@	{
//@	"dependencies_extra":[{"ref":"./image_io.o", "rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_IMAGEIO_HPP
#define TERRAFORMER_IMAGEIO_HPP

#include "./image.hpp"
#include "./ilm_io.hpp"

#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfInputFile.h>

namespace terraformer
{

	namespace image_io_detail
	{
		template<class T>
		using empty = std::type_identity<T>;
		using output_file_factory = Imf::OutputFile (*)(void*, Imf::Header const&);
		using input_file_factory  = Imf::InputFile (*)(void*);
	}

	image load(image_io_detail::empty<image>,
		void* arg,
		image_io_detail::input_file_factory make_input_file);

	inline image load(image_io_detail::empty<image>, char const* filename)
	{
		return load(image_io_detail::empty<image>{}, const_cast<char*>(filename), [](void* filename) {
			return Imf::InputFile{static_cast<char const*>(filename)};
		});
	}

	template<class FileReader>
	image load(image_io_detail::empty<image>, FileReader&& reader)
	{
		ilm_input_adapter input{std::forward<FileReader>(reader)};
		return load(image_io_detail::empty<image>{}, &input, [](void* input) {
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
	void store(span_2d<rgba_pixel const> pixels, FileWriter&& writer)
	{
		ilm_output_adapter output{std::forward<FileWriter>(writer)};
		store(pixels, &output, [](void* output, Imf::Header const& header) {
			return Imf::OutputFile{*static_cast<ilm_output_adapter<FileWriter>*>(output), header};
		});
	}

	template<class FileWriter>
	void store(image const& img, FileWriter&& writer)
	{
		store(img.pixels(), std::forward<FileWriter>(writer));
	}

	grayscale_image load(image_io_detail::empty<grayscale_image>,
		void* arg,
		image_io_detail::input_file_factory make_input_file);

	inline grayscale_image load(image_io_detail::empty<grayscale_image>, char const* filename)
	{
		return load(image_io_detail::empty<grayscale_image>{}, const_cast<char*>(filename), [](void* filename) {
			return Imf::InputFile{static_cast<char const*>(filename)};
		});
	}

	template<class FileReader>
	grayscale_image load(image_io_detail::empty<grayscale_image>, FileReader&& reader)
	{
		ilm_input_adapter input{std::forward<FileReader>(reader)};
		return load(image_io_detail::empty<grayscale_image>{}, &input, [](void* input) {
			return Imf::InputFile{*static_cast<ilm_input_adapter<FileReader>*>(input)};
		});
	}
}

#endif