#ifndef TERRAFORMER_COMPOSITE_FUNCTION_HPP
#define TERRAFORMER_COMPOSITE_FUNCTION_HPP

#include <tuple>

namespace terraformer
{
template<class ... Functions>
class composite_function
{
public:
	constexpr explicit composite_function(Functions&&... functions):
			m_functions{std::move(functions)...}
	{}

	template<class Arg>
	constexpr decltype(auto) operator()(Arg&& x) const
	{
		return std::apply([input = std::forward<Arg>(x)](auto& first, auto&... other) mutable {
			return apply(std::forward<Arg>(input), first, other...);
		}, m_functions);
	}

	template<class Arg>
	constexpr decltype(auto) operator()(Arg&& x)
	{
		return std::apply([input = std::forward<Arg>(x)](auto& first, auto&... other) mutable {
			return apply(std::forward<Arg>(input), first, other...);
		}, m_functions);
	}

private:
	template<class Arg, class Func>
	static constexpr decltype(auto) apply(Arg&& arg, Func&& f)
	{
			return std::forward<Func>(f)(std::forward<Arg>(arg));
	}

	template<class Arg, class First, class ... T>
	static constexpr decltype(auto) apply(Arg&& x, First&& first, T&&... other)
	{
			return apply(std::forward<First>(first)(std::forward<Arg>(x)), std::forward<T>(other)...);
	}

	std::tuple<Functions...> m_functions;
};
}

#endif