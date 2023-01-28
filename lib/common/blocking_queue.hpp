#ifndef TERRAFORMER_DIFFUSER_BLOCKING_QUEUE_HPP
#define TERRAFORMER_DIFFUSER_BLOCKING_QUEUE_HPP

#include <mutex>
#include <queue>

namespace terraformer
{
	template<class T>
	class blocking_queue
	{
	public:
		void push(T&& cb)
		{
			std::lock_guard lock{m_mutex};
			m_queue.push(std::move(cb));
		}

		std::optional<T> try_pop()
		{
			std::lock_guard lock{m_mutex};
			if(m_queue.empty())
			{ return std::nullopt; }

			auto ret = std::move(m_queue.front());
			m_queue.pop();
			return ret;
		}

	private:
		std::mutex m_mutex;
		std::queue<T> m_queue;
	};
}

#endif