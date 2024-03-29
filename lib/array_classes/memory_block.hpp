#ifndef TERRAFORMER_MEMORY_BLOCK_HPP
#define TERRAFORMER_MEMORY_BLOCK_HPP

#include "./array_index.hpp"

#include <memory>
#include <cstdlib>

namespace terraformer
{
	class memory_block
	{
	public:
		memory_block() = default;

		explicit memory_block(byte_size<size_t> capacity):
			m_pointer{malloc(capacity.get())}
		{
			if(m_pointer == nullptr)
			{ throw std::runtime_error{"Failed to allocate memory"}; }
		}

		[[nodiscard]] void* get() const noexcept
		{ return m_pointer.get(); }

		template<class T>
		[[nodiscard]] T* interpret_as() const noexcept
		{ return reinterpret_cast<T*>(m_pointer.get()); }

		[[nodiscard]] operator bool() const noexcept
		{ return m_pointer != nullptr; }

	private:
		struct deleter
		{
			void operator()(void* ptr) const
			{ free(ptr); }
		};
		std::unique_ptr<void, deleter> m_pointer;
	};
};

#endif
