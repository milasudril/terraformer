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

		size_t max_concurrency() const { return std::size(m_workers); }

		[[deprecated("Use max_concurrency instead")]]
		size_t size() const { return max_concurrency(); }

		void submit(Task&& task)
		{
			m_tasks.push(std::move(task));
			std::lock_guard lock{m_mutex};
			m_cv.notify_all();
		}

		[[deprecated("Use submit instead")]]
		void run(Task&& task)
		{ submit(std::move(task)); }

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

		void dispatch()
		{
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

	struct thread_pool_factory
	{
		size_t num_workers;

		template<class Task>
		auto operator()(empty<Task>) const
		{
			return thread_pool<Task>{num_workers};
		}
	};

	template<class Task>
	class single_thread_pool_manager
	{
	public:
		using task_type = Task;

		explicit single_thread_pool_manager(size_t num_workers):m_thread_pool{num_workers}{}

		auto& operator()(empty<Task>)
		{
			return m_thread_pool;
		}

	private:
		thread_pool<Task> m_thread_pool;
	};

	template<class ThreadPoolFactory>
	class thread_pool_factory_adaptor:private ThreadPoolFactory
	{
	public:
		using ThreadPoolFactory::ThreadPoolFactory;

		template<class Task>
		decltype(auto) operator()(empty<Task>)
		{ return ThreadPoolFactory::operator()(empty<typename ThreadPoolFactory::task_type>{}); }
	};

	using default_thread_pool =
		thread_pool_factory_adaptor<single_thread_pool_manager<std::function<void()>>>;
}
#endif