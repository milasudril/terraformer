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
		class impl
		{
		public:
			explicit impl(size_t start_value):m_value{start_value}{}

			void wait() const
			{
				std::unique_lock lock{m_mutex};
				return m_cv.wait(lock, [this](){ return m_value == 0; });
			}

			void decrement()
			{
				std::lock_guard lock{m_mutex};
				--m_value;
				if(m_value == 0)
				{ m_cv.notify_one(); }
			}

			void wait_and_reset(size_t new_value)
			{
				std::unique_lock lock{m_mutex};
				m_cv.wait(lock, [this](){ return m_value == 0; });
				m_value = new_value;
			}

		private:
			size_t m_value;
			mutable std::mutex m_mutex;
			mutable std::condition_variable m_cv;
		};

	public:
		using semaphore = impl;

		explicit signaling_counter(size_t start_value):
			m_ctrl{std::make_unique<impl>(start_value)}
		{}

		signaling_counter(signaling_counter&& other) = default;
		signaling_counter& operator=(signaling_counter&& other) = default;
		signaling_counter(signaling_counter const& other) = delete;
		signaling_counter& operator=(signaling_counter const& other) = delete;

		void wait() const
		{ m_ctrl->wait(); }

		~signaling_counter()
		{
			if(m_ctrl != nullptr)
			{ wait(); }
		}

		auto& get_state()
		{ return *m_ctrl; }

	private:
		std::unique_ptr<impl> m_ctrl;
	};

	inline void trigger(signaling_counter::semaphore& counter)
	{ counter.decrement(); }
}

#endif