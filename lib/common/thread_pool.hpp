#ifndef TERRAFORMER_DIFFUSER_THREADPOOL_HPP
#define TERRAFORMER_DIFFUSER_THREADPOOL_HPP

#include "./blocking_queue.hpp"

#include <thread>
#include <condition_variable>

namespace terraformer
{
	template<class Task>
	class thread_pool
	{
	public:
		explicit thread_pool(size_t num_workers):m_should_stop{false}
		{
			std::generate_n(std::back_inserter(m_workers), num_workers, [this](){
				return std::thread{[this](){dispatch();}};
			});
		}

		size_t size() const { return std::size(m_workers); }

		void schedule(Task&& task)
		{
			m_tasks.push(std::move(task));
			std::lock_guard lock{m_mutex};
			m_cv.notify_all();
		}

		void terminate()
		{
			{
				std::lock_guard lock{m_mutex};
				m_should_stop = true;
				m_cv.notify_all();
			}

			std::ranges::for_each(m_workers, [](auto& item){
				item.join();
			});
		}

		~thread_pool()
		{ terminate(); }

	private:
		blocking_queue<Task> m_tasks;
		bool m_should_stop;

		void dispatch()
		{
			while(true)
			{
				bool should_stop{};
				std::optional<Task> t{};
				{
					std::unique_lock lock{m_mutex};
					m_cv.wait(lock, [this, &t](){
						t = m_tasks.try_pop();
						return t.has_value() || m_should_stop;
					});
					should_stop = m_should_stop;
				}
				if(should_stop)
				{ return; }

				if(t.has_value())
				{ (*t)(); }
			}
		}

		std::mutex m_mutex;
		std::condition_variable m_cv;
		std::vector<std::thread> m_workers;
	};
}
#endif