#ifndef TERRAFORMER_BATCH_RESULT_HPP
#define TERRAFORMER_BATCH_RESULT_HPP

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>

namespace terraformer
{
	template<class ResultType>
	class batch_result_state
	{
	public:
		explicit batch_result_state(size_t num_tasks_to_complete):
			m_num_tasks_to_complete{num_tasks_to_complete}
		{}

		template<class FoldOperation>
		[[nodiscard]] auto get_result(FoldOperation&& fold)
		{
			std::unique_lock lock{m_mutex};
			m_cv.wait(lock, [this](){
				return m_num_tasks_to_complete == std::size(m_received_results);
			});

			return std::forward<FoldOperation>(fold)(std::move(m_received_results));
		}

		template<class... Args>
		void save_partial_result(Args&&... args)
		{
			std::lock_guard lock{m_mutex};
			if(std::size(m_received_results) == m_num_tasks_to_complete) [[unlikely]]
			{ m_received_results.clear(); }

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

	template<>
	class batch_result_state<void>
	{
		public:
		explicit batch_result_state(size_t num_tasks_to_complete):
			m_num_tasks_to_complete{num_tasks_to_complete}
		{}

		void wait() const
		{
			std::unique_lock lock{m_mutex};
			return m_cv.wait(lock, [this](){ return m_num_tasks_to_complete == 0; });
		}

		void mark_batch_as_completed()
		{
			std::lock_guard lock{m_mutex};
			--m_num_tasks_to_complete;
			if(m_num_tasks_to_complete == 0)
			{ m_cv.notify_one(); }
		}

		private:
			size_t m_num_tasks_to_complete;
			mutable std::mutex m_mutex;
			mutable std::condition_variable m_cv;
	};

	template<class ResultType>
	class batch_result
	{
	public:
		explicit batch_result(size_t num_tasks_to_complete):
			m_state{std::make_unique<batch_result_state<ResultType>>(num_tasks_to_complete)}
		{}

		template<class FoldOperation>
		requires(!std::is_same_v<ResultType, void>)
		[[nodiscard]] auto get_result(FoldOperation&& fold) const
		{ return m_state->get_result(std::forward<FoldOperation>(fold)); }

		template<class T = void>
		requires(std::is_same_v<ResultType, void>)
		void wait() const
		{ m_state->wait(); }

		auto& get_state()
		{ return *m_state; }

	private:
		std::unique_ptr<batch_result_state<ResultType>> m_state;
	};
}
#endif