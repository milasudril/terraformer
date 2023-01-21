#ifndef TERRAFORMER_DIFFUSER_NOTIFYING_TASK_HPP
#define TERRAFORMER_DIFFUSER_NOTIFYING_TASK_HPP

#include "./callable_tuple.hpp"

namespace terraformer
{
	template<class Event, class Callable, class ... Args>
	class notifying_task
	{
	public:
		template<class ... T>
		explicit notifying_task(Event&& event, Callable&& func, T&& ... args):
			m_task{std::move(func), std::forward<T>(args)...},
			m_event{std::move(event)}
		{}

		auto operator()()
		{
			if constexpr(std::is_same_v<decltype(m_task()), void>)
			{
				m_task();
				trigger(m_event);
			}
			else
			{
				auto ret = m_task();
				trigger(m_event);
				return ret;
			}
		}

	private:
		callable_tuple<Callable, Args ...> m_task;
		Event m_event;
	};

	template<class Event, class Callable, class ... T>
	notifying_task(Event&&, Callable&&, T&& ...)
		-> notifying_task<Event, Callable, std::remove_cvref_t<T>...>;
}

#endif