#ifndef TERRAFORMER_BATCH_RESULT_HPP
#define TERRAFORMER_BATCH_RESULT_HPP

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

namespace terraformer
{
	template<class ResultType>
	class batch_result
	{
	public:
		template<class Fold>
		[[nodiscard]] auto get_result(Fold&& fold) const
		{
			std::unique_lock lock{m_mutex};
			m_cv.wait(lock, [this](){
				return m_num_completed_tasks == std::size(m_received_result);
			});

			return std::forward<Fold>(fold)(m_received_results);
		}

		template<class... Args>
		void save_partial_result(Args&&... args)
		{
			std::lock_guard lock{m_mutex}
			m_received_results.emplace_back(std::forward<Args>(args)...);
			if(std::size(m_received_results) == m_num_tasks_to_complete)
			{ m_cv.notify_one(); }
		}

	private:
		std::vector<ResultType> m_received_results;
		size_t m_num_tasks_to_complete{0};
		mutable std::mutex m_mutex;
		mutable std::condition_variable m_cv;
	};
}
#endif