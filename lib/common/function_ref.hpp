#ifndef TERRAFORMER_FUNCTION_REF_HPP
#define TERRAFORMER_FUNCTION_REF_HPP

#include <type_traits>
#include <utility>

namespace terraformer
{
	template<class...>
	class function_ref;

	struct no_operation_tag{};

	template<auto Callable>
	struct bound_callable
	{
		template<class... Args>
		static constexpr decltype(auto) call(Args&&... args)
		{ return Callable(std::forward<Args>(args)...); }
	};

	template<class R, class... Args>
	class function_ref<R(Args...)>
	{
	public:
		using stateless_callback = R (*)(Args...);

		function_ref() = default;

		function_ref(no_operation_tag)
			requires(std::is_same_v<std::remove_cv_t<R>, void>):
			m_handle{nullptr},
			m_function{[](void*, Args...){}}
		{}

		function_ref(stateless_callback cb):
			m_handle{reinterpret_cast<void*>(cb)},
			m_function{[](void* handle, Args... args){
				auto cb = reinterpret_cast<stateless_callback>(handle);
				return cb(std::forward<Args>(args)...);
			}}
		{}

		template<stateless_callback Callable>
		function_ref(bound_callable<Callable>):
			m_handle{nullptr},
			m_function{[](void*, Args... args){
				return bound_callable<Callable>::call(std::forward<Args>(args)...);
			}}
		{
		}

		template<class Func>
		function_ref(std::reference_wrapper<Func> f):
			m_handle{&f.get()},
			m_function{[](void* handle, Args... args){
				auto& obj = *static_cast<Func*>(handle);
				return obj(std::forward<Args>(args)...);
			}}
		{}

		template<class Func>
		function_ref(Func* f):
			m_handle{f},
			m_function{[](void* handle, Args... args){
				auto& obj = *static_cast<Func*>(handle);
				return obj(std::forward<Args>(args)...);
			}}
		{}

		R operator()(Args... args) const
		{ return m_function(m_handle, std::forward<Args>(args)...); }

		operator bool() const
		{ return m_function != nullptr; }

		void* handle() const
		{ return m_handle; }

	private:
		static void empty_dtor(void*){}
		void* m_handle = nullptr;
		R (*m_function)(void*, Args... args) = nullptr;
	};
}

#endif