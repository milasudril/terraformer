#ifndef TERRAFORMER_MOVE_ONLY_FUNCTION_HPP
#define TERRAFORMER_MOVE_ONLY_FUNCTION_HPP

#include <type_traits>
#include <utility>

namespace terraformer
{
	template<class...>
	class move_only_function;

	struct no_operation_tag{};

	template<auto Callable>
	struct bound_callable
	{
		template<class... Args>
		static constexpr decltype(auto) call(Args&&... args)
		{ return Callable(std::forward<Args>(args)...); }
	};

	// TODO: C++23: Replace with std with the benefit of const correctness
	template<class R, class... Args>
	class move_only_function<R(Args...)>
	{
	public:
		using stateless_callback = R (*)(Args...);

		move_only_function() = default;

		move_only_function(no_operation_tag)
			requires(std::is_same_v<std::remove_cv_t<R>, void>):
			m_handle{nullptr},
			m_function{[](void*, Args...){}},
			m_dtor{empty_dtor}
		{}

		move_only_function(std::nullptr_t) noexcept: move_only_function{}
		{}

		move_only_function(move_only_function&& other) noexcept:
			m_handle{std::exchange(other.m_handle, nullptr)},
			m_function{std::exchange(other.m_function, nullptr)},
			m_dtor{std::exchange(other.m_dtor, empty_dtor)}
		{ }

		move_only_function(const move_only_function&) = delete;

		move_only_function& operator=(move_only_function&& other) noexcept
		{
			reset();
			std::swap(other.m_handle, m_handle);
			std::swap(other.m_function, m_function);
			std::swap(other.m_dtor, m_dtor);
			return *this;
		};

		void reset()
		{
			m_dtor(m_handle);
			m_handle = nullptr;
			m_function = nullptr;
			m_dtor = empty_dtor;
		}

		move_only_function& operator=(move_only_function const&) = delete;

		~move_only_function()
		{ m_dtor(m_handle); }

		move_only_function(stateless_callback cb):
			m_handle{reinterpret_cast<void*>(cb)},
			m_function{[](void* handle, Args... args){
				auto cb = reinterpret_cast<stateless_callback>(handle);
				return cb(std::forward<Args>(args)...);
			}},
			m_dtor{empty_dtor}
		{}

		template<stateless_callback Callable>
		move_only_function(bound_callable<Callable>):
			m_handle{nullptr},
			m_function{[](void*, Args... args){
				return bound_callable<Callable>::call(std::forward<Args>(args)...);
			}},
			m_dtor{empty_dtor}
		{
		}

		template<class Func>
		requires(
			!std::is_same_v<std::remove_cvref_t<Func>, move_only_function>
			&& !std::is_same_v<std::decay_t<Func>, stateless_callback>
		)
		move_only_function(Func&& f):
			m_handle{new std::remove_cvref_t<Func>(std::forward<Func>(f))},
			m_function{[](void* handle, Args... args){
				auto& obj = *static_cast<std::remove_cvref_t<Func>*>(handle);
				return obj(std::forward<Args>(args)...);
			}},
			m_dtor{[](void* handle){
				auto obj = static_cast<std::remove_cvref_t<Func>*>(handle);
				delete obj;
			}}
		{}

		R operator()(Args... args) const
		{ return m_function(m_handle, std::forward<Args>(args)...); }

		operator bool() const
		{ return m_function != nullptr; }

	private:
		static void empty_dtor(void*){}
		void* m_handle = nullptr;
		R (*m_function)(void*, Args... args) = nullptr;
		void (*m_dtor)(void*) = empty_dtor;
	};


}

#endif