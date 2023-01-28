#ifndef TERRAFORMER_DIFFUSER_SIGNALING_COUNTER_HPP
#define TERRAFORMER_DIFFUSER_SIGNALING_COUNTER_HPP

#include <cstddef>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace terraformer
{
	class signaling_counter
	{
		class control_block
		{
		public:
			template<class Pred>
			decltype(auto) wait(Pred&& pred)
			{
				std::unique_lock lock{m_mutex};
				return m_cv.wait(lock, std::forward<Pred>(pred));
			}

			template<class Action>
			void notify_if_successful(Action&& action)
			{
				std::lock_guard lock{m_mutex};
				if(std::forward<Action>(action)())
				{ m_cv.notify_one(); }
			}

		private:
			std::mutex m_mutex;
			std::condition_variable m_cv;
		};
	public:
		explicit signaling_counter(size_t start_value):
			m_value{start_value},
			m_ctrl{std::make_unique<control_block>()}
		{}

		signaling_counter(signaling_counter&& other) noexcept:
			m_value{other.m_value},
			m_ctrl{std::move(other.m_ctrl)}
		{}

		signaling_counter& operator=(signaling_counter&& other) noexcept
		{
			m_value = other.m_value;
			m_ctrl = std::exchange(other.m_ctrl, std::move(m_ctrl));
			return *this;
		}

		void decrement()
		{
			m_ctrl->notify_if_successful([this]() {
				--m_value;
				return m_value == 0;
			});
		}

		void wait()
		{
			m_ctrl->wait([this](){ return m_value == 0; });
		}

		~signaling_counter()
		{
			if(m_ctrl != nullptr)
			{ wait(); }
		}

	private:
		size_t m_value;
		std::unique_ptr<control_block> m_ctrl;
	};

	inline void trigger(signaling_counter& counter)
	{ counter.decrement(); }
}

#endif