#ifndef TERRAFORMER_DIFFUSER_SIGNALING_COUNTER_HPP
#define TERRAFORMER_DIFFUSER_SIGNALING_COUNTER_HPP

#include <cstddef>
#include <mutex>
#include <condition_variable>

namespace terraformer
{
	class signaling_counter
	{
	public:
		explicit signaling_counter(size_t start_value):m_value{start_value}{}

		void decrement()
		{
			std::lock_guard lock{m_mutex};
			--m_value;
			if(m_value == 0)
			{ m_cv.notify_one(); }
		}

		void wait()
		{
			std::unique_lock lock{m_mutex};
			m_cv.wait(lock, [this](){ return m_value == 0; });
		}

		~signaling_counter()
		{ wait(); }

	private:
		size_t m_value;
		std::mutex m_mutex;
		std::condition_variable m_cv;
	};

	inline void trigger(signaling_counter& counter)
	{ counter.decrement(); }
}

#endif