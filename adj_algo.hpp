#ifndef TERRAFORMER_ADJ_ALGO_HPP
#define TERRAFORMER_ADJ_ALGO_HPP

#include <utility>

template<class Iter, class Func>
void adj_for_each(Iter begin, Iter end, Func&& f)
{
	if(begin == end)
	{
		return;
	}

	auto next = begin;
	++next;
	while(next != end)
	{
		f(*begin, *next);
		++begin;
		++next;
	}
}

namespace adj_algo_detail
{
	template<class Iter>
	struct IterValueType
	{
		using type = std::remove_reference_t<decltype(*std::declval<Iter>())>;
	};

	template<class Iter, class Func>
	struct ReturnType
	{
		using type = decltype(
			std::declval<Func>()
				(std::declval<IterValueType<Iter>::type>(), std::declval<IterValueType<Iter>::type>()));
	};
}

template<class Iter, class Func, class T>
auto adj_integrate(Iter begin, Iter end, Func&& f, typename adj_algo_detail::ReturnType<Iter, Func>::type&& init = 0)
{
	adj_for_each(begin, end, [&init, func=std::forward<T>(f)](auto a, auto b) mutable {
		init += func(a, b);
	});
	return init;
}

template<class Input, class Output, class Func>
void adj_transform(Input begin, Input end, Output&& output, Func&& f)
{
	adj_for_each(begin, end, [o = std::forward<Output>(output), func = std::forward<Func>(f)](auto a, auto b) mutable {
		*o = func(a, b);
		++o;
	});
}

#endif