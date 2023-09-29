#ifndef TERRAFORMER_TASK_RECEIVER_HPP
#define TERRAFORMER_TASK_RECEIVER_HPP

#include <thread>
#include <condition_variable>
#include <optional>

namespace terraformer
{
	template<class Task>
	class task_receiver
	{
	public:
		explicit task_receiver():m_should_stop{false}, m_worker{[this](){dispatch();}}
		{}

		void replace_pending_task(Task&& task)
		{
			std::lock_guard lock{m_mutex};
			m_task = std::move(task);
			m_cv.notify_one();
		}

		void terminate()
		{
			{
				std::lock_guard lock{m_mutex};
				m_should_stop = true;
				m_cv.notify_one();
			}
			m_worker.join();
		}

		~task_receiver()
		{ terminate(); }

	private:
		std::optional<Task> m_task;
		bool m_should_stop;

		void dispatch()
		{
			while(true)
			{
				bool should_stop{};
				std::optional<Task> t{};
				{
					std::unique_lock lock{m_mutex};
					m_cv.wait(lock, [this](){
						return m_task.has_value() || m_should_stop;
					});
					should_stop = m_should_stop;
					t = std::move(m_task);
					m_task.reset();
				}
				if(should_stop)
				{ return; }

				if(t.has_value())
				{
					try
					{ (*t)(); }
					catch(std::exception const& err)
					{
						fprintf(stderr, "(x) %s\n", err.what());
						fflush(stderr);
					}
				}
			}
		}

		std::mutex m_mutex;
		std::condition_variable m_cv;
		std::thread m_worker;
	};
}
#endif