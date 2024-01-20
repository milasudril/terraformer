#ifndef TERRAFORMER_TEMPDIR_HPP
#define TERRAFORMER_TEMPDIR_HPP

#include <cstdlib>
#include <filesystem>

namespace terraformer
{
	class tempdir
	{
	public:
		explicit tempdir(std::string_view template_string)
		{
			std::string buffer{template_string};
			auto res = mkdtemp(std::data(buffer));

			if(res == nullptr)
			{ throw std::runtime_error{"Failed to create temp dir"}; }

			m_name = res;
			m_keep_after_scope = false;
		}

		tempdir& keep_after_scope(bool value)
		{
			m_keep_after_scope = value;
			return *this;
		}

		~tempdir()
		{
			if(!m_keep_after_scope)
			{ remove_all(m_name); }
		}

		auto const& get_name() const&
		{ return m_name; }

	private:
		std::filesystem::path m_name;
		bool m_keep_after_scope;
	};
}

#endif
