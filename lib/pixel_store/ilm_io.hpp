//	{
//	 "dependencies":[{"ref":"OpenEXR", "origin":"pkg-config"}]
//	}

#ifndef TERRAFORMER_ILMIO_HPP
#define TERRAFORMER_ILMIO_HPP

#include <OpenEXR/ImfIO.h>

#include <span>

namespace terraformer
{
	namespace detail
	{
		template<class T>
		void write(T& handle, char const* buffer, int n)
		{
			(void)write(handle, std::as_bytes(std::span{buffer, static_cast<size_t>(n)}));
		}

		template<class T>
		bool read(T& handle, char* buffer, int n)
		{
			auto n_read =
				read(handle, std::as_writable_bytes(std::span{buffer, static_cast<size_t>(n)}));
			if(n_read == 0) { return false; }

			if(n_read != static_cast<size_t>(n)) { throw "Unexpected end of file"; }

			return true;
		}

		template<class T>
		uint64_t tellp(T& handle)
		{
			return tell(handle);
		}

		template<class T>
		void seekp(T& handle, uint64_t pos)
		{
			(void)seek(handle, pos);
		}
	}

	template<class FileWriter>
	class ilm_output_adapter: public Imf::OStream
	{
	public:
		explicit ilm_output_adapter(FileWriter&& writer)
			: Imf::OStream{"Dummy"}
			, m_writer{std::move(writer)}
		{
		}

		void write(char const* buffer, int n) override { detail::write(m_writer, buffer, n); }

		uint64_t tellp() override { return detail::tellp(m_writer); }

		void seekp(uint64_t pos) override { detail::seekp(m_writer, pos); }

	private:
		FileWriter m_writer;
	};

	template<class FileReader>
	class ilm_input_adapter: public Imf::IStream
	{
	public:
		explicit ilm_input_adapter(FileReader&& reader)
			: Imf::IStream{"Dummy"}
			, m_reader{std::move(reader)}
		{
		}

		bool read(char* buffer, int n) override { return detail::read(m_reader, buffer, n); }

		uint64_t tellg() override { return detail::tellp(m_reader); }

		void seekg(uint64_t pos) override { detail::seekp(m_reader, pos); }

		void clear() override {}

	private:
		FileReader m_reader;
	};
}
#endif