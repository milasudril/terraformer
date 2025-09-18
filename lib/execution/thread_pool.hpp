#ifndef TERRAFORMER_DIFFUSER_THREADPOOL_HPP
#define TERRAFORMER_DIFFUSER_THREADPOOL_HPP

#include "./blocking_queue.hpp"

#include <thread>
#include <condition_variable>
#include <algorithm>

namespace terraformer
{
	template<class Task>
	class thread_pool
	{
	public:
		explicit thread_pool(size_t num_workers):m_should_stop{false}
		{
			std::generate_n(std::back_inserter(m_workers), num_workers, [this, n = size_t{0}]() mutable{
				auto worker_index = n;
				++n;
				return std::thread{[this, worker_index](){dispatch(worker_index);}};
			});
		}

		size_t max_concurrency() const { return std::size(m_workers); }

		static size_t current_worker()
		{ return m_current_worker; }

		void submit(Task&& task)
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

		template<class SchedParams>
		void set_schedparams(SchedParams const& params)
		{
			for(auto& thread:m_workers)
			{ params.apply(thread.native_handle()); }
		}

		~thread_pool()
		{ terminate(); }

	private:
		blocking_queue<Task> m_tasks;
		bool m_should_stop;

		static inline thread_local size_t m_current_worker = -1;

		void dispatch(size_t worker_index)
		{
			m_current_worker = worker_index;

			while(true)
			{
				bool should_stop{};
				std::optional<Task> t{};
				{
					std::unique_lock lock{m_mutex};
					m_cv.wait(lock, [this, &t](){
						t = std::move(m_tasks.try_pop());
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