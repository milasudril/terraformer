#ifndef TERRAFORMER_MOVE_ONLY_FUNCTION_HPP
#define TERRAFORMER_MOVE_ONLY_FUNCTION_HPP

#include "./function_ref.hpp"

#include <type_traits>
#include <utility>

namespace terraformer
{
	template<class...>
	class move_only_function;

	// TODO: C++23: Replace with std with the benefit of const correctness
	template<class R, class... Args>
	class move_only_function<R(Args...)>:private function_ref<R(Args...)>
	{
	public:
		using base = function_ref<R(Args...)>;
		using stateless_callback = typename base::stateless_callback;
		using base::operator();
		using base::operator bool;

		move_only_function() = default;

		move_only_function(no_operation_tag)
			requires(std::is_same_v<std::remove_cv_t<R>, void>):
			base{no_operation_tag{}},
			m_dtor{empty_dtor}
		{}

		move_only_function(std::nullptr_t) noexcept: move_only_function{}
		{}

		move_only_function(move_only_function&& other) noexcept:
			base{std::exchange(static_cast<base&>(other), base{})},
			m_dtor{std::exchange(other.m_dtor, empty_dtor)}
		{ }

		move_only_function(const move_only_function&) = delete;

		move_only_function& operator=(move_only_function&& other) noexcept
		{
			reset();
			std::swap(static_cast<base&>(other), static_cast<base&>(*this));
			std::swap(other.m_dtor, m_dtor);
			return *this;
		};

		void reset()
		{
			m_dtor(static_cast<base&>(*this).handle());
			static_cast<base&>(*this) = base{};
			m_dtor = empty_dtor;
		}

		move_only_function& operator=(move_only_function const&) = delete;

		~move_only_function()
		{ m_dtor(static_cast<base&>(*this).handle()); }

		move_only_function(stateless_callback cb):
			base{cb},
			m_dtor{empty_dtor}
		{}

		template<stateless_callback Callable>
		move_only_function(bound_callable<Callable>):
			base{bound_callable<Callable>{}},
			m_dtor{empty_dtor}
		{
		}

		template<class Func>
		requires(
			!std::is_same_v<std::remove_cvref_t<Func>, move_only_function>
			&& !std::is_same_v<std::decay_t<Func>, stateless_callback>
		)
		move_only_function(Func&& f):
			base{new std::remove_cvref_t<Func>(std::forward<Func>(f))},
			m_dtor{[](void* handle){
				auto obj = static_cast<std::remove_cvref_t<Func>*>(handle);
				delete obj;
			}}
		{}

	private:
		static void empty_dtor(void*){}
		void (*m_dtor)(void*) = empty_dtor;
	};


}

#endif