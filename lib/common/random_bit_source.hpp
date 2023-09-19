//@	{"dependencies_extra":[{"ref": "./random_bit_source.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RANDOM_BIT_SOURCE_HPP
#define TERRAFORMER_RANDOM_BIT_SOURCE_HPP

#include <unistd.h>
#include <fcntl.h>
#include <span>

namespace terraformer
{
	class random_bit_source
	{
	public:
		random_bit_source():m_fd{open("/dev/urandom", O_RDONLY)}
		{}
		
		random_bit_source(random_bit_source&& other):m_fd{other.m_fd}
		{ other.m_fd = -1; }
		
		random_bit_source& operator=(random_bit_source&& other)
		{
			m_fd = other.m_fd;
			other.m_fd = -1;
			return *this;
		}
		
		void read(std::span<std::byte> buffer) const;
		
		~random_bit_source()
		{
			if(m_fd != -1)
			{ close(m_fd); }
		}
		
	private:
		int m_fd;
	};
};

#endif
