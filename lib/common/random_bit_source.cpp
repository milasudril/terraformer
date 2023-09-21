//@	{"target":{"name":"random_bit_source.o"}}

#include "./random_bit_source.hpp"

#include <cerrno>
#include <cstdlib>

void terraformer::random_bit_source::read(std::span<std::byte> buffer) const
{
	auto bytes_to_read = std::size(buffer);
	auto write_ptr = std::data(buffer);
	while(bytes_to_read != 0)
	{
		auto const n = ::read(m_fd, write_ptr, bytes_to_read);
		if(n == -1)
		{
			if(errno != EAGAIN || errno != EWOULDBLOCK)
			{ abort(); }
			continue;
		}
		
		bytes_to_read -= n;
		write_ptr += n;
	}
}
