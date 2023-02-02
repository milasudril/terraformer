#ifndef TERRAFORMER_DIFFUSER_CALLABLE_TUPLE_HPP
#define TERRAFORMER_DIFFUSER_CALLABLE_TUPLE_HPP

#include <tuple>

namespace terraformer
{
	template<class Callable, class ... Args>
	class callable_tuple
	{
	public:
		template<class ... T>
		explicit callable_tuple(Callable&& func, T&& ... args):
			m_func{std::move(func)}, m_args{std::forward<T>(args)...}
		{}

		decltype(auto) operator()() const
		{ return std::apply(m_func, m_args); }

	private:
		Callable m_func;
		[[no_unique_address]] std::tuple<Args...> m_args;
	};

	template<class Callable, class ... T>
	callable_tuple(Callable&&, T&& ...) -> callable_tuple<Callable, std::remove_cvref_t<T>...>;
}

#endif