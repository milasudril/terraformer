//@	{"dependencies_extra":[{"ref":"./rng.o","rel":"implementation"}]}

#ifndef TERRAFORMER_RNG_HPP
#define TERRAFORMER_RNG_HPP

#include <pcg-cpp/include/pcg_random.hpp>

#include <unistd.h>
#include <fcntl.h>

#include <span>
#include <filesystem>

namespace terraformer
{
	using random_generator = pcg_engines::oneseq_dxsm_128_64;
	using rng_seed_type = __int128 unsigned;

	constexpr char const* system_rng_path = "/dev/urandom";

	class random_bit_source
	{
	public:
		random_bit_source(std::filesystem::path const& rng_path):m_fd{open(rng_path.c_str(), O_RDONLY)}
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

		template<class T>
		requires(std::is_trivial_v<T>)
		T get() const
		{
			T ret;
			read(std::as_writable_bytes(std::span{&ret, 1}));
			return ret;
		}

		~random_bit_source()
		{
			if(m_fd != -1)
			{ close(m_fd); }
		}

	private:
		int m_fd;
	};

	inline rng_seed_type generate_rng_seed(random_generator& rng)
	{
		static_assert(std::is_same_v<random_generator::result_type, size_t>);
		auto const val_1 = static_cast<rng_seed_type>(rng());
		auto const val_2 = static_cast<rng_seed_type>(rng());
		return (val_1 << static_cast<rng_seed_type>(64)) | val_2;
	}

	inline rng_seed_type generate_rng_seed(std::filesystem::path const& rng_path)
	{ return random_bit_source{rng_path}.get<rng_seed_type>(); }
}

#endif