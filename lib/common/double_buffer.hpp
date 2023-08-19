#ifndef TERRAFORMER_DOUBLE_BUFFER_HPP
#define TERRAFORMER_DOUBLE_BUFFER_HPP

#include <utility>

namespace terraformer
{
	template<class BufferType>
	class double_buffer
	{
	public:
		template<class First, class ... Args>
		requires(!std::is_same_v<std::decay_t<First>, double_buffer>)
		explicit double_buffer(First const& arg, Args const&... args):
			m_a{arg, args ...},
			m_b{arg, args ...}
		{}

		BufferType const& front() const
		{ return m_a; }

		BufferType& back()
		{ return m_b; }

		void swap()
		{
			using std::swap;
			swap(m_a, m_b);
		}

	private:
		BufferType m_a;
		BufferType m_b;
	};
}

#endif