#ifndef TERRAFORMER_DIFFUSER_BLOCKING_QUEUE_HPP
#define TERRAFORMER_DIFFUSER_BLOCKING_QUEUE_HPP

#include <shared_mutex>
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
			write_lock lock{m_mutex};
			m_queue.push(std::move(cb));
		}

		std::optional<T> try_pop()
		{
			write_lock lock{m_mutex};
			if(m_queue.empty())
			{ return std::nullopt; }

			auto ret = m_queue.front();
			m_queue.pop();
			return std::move(ret);
		}

	private:
		using read_lock = std::unique_lock<std::shared_mutex>;
		using write_lock = std::shared_lock<std::shared_mutex>;
		std::shared_mutex m_mutex;
		std::queue<T> m_queue;
	};
}

#endif