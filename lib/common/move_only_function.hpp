#ifndef TERRAFORMER_MOVE_ONLY_FUNCTION_HPP
#define TERRAFORMER_MOVE_ONLY_FUNCTION_HPP

namespace terraformer
{
	template<class...>
	class move_only_function;

	struct no_operation_tag{};

	template<class R, class... Args>
	class move_only_function<R(Args...)>
	{
	public:
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

		template<class Func>
		requires(!std::is_same_v<std::remove_cvref_t<Func>, move_only_function>)
		move_only_function(Func&& f):
			m_handle{new Func(std::forward<Func>(f))},
			m_function{[](void* handle, Args... args){
				auto& obj = *static_cast<Func*>(handle);
				return obj(std::forward<Args>(args)...);
			}},
			m_dtor{[](void* handle){
				auto obj = static_cast<Func*>(handle);
				delete obj;
			}}
		{}

		R operator()(Args... args)
		{ return m_function(m_handle, std::forward<Args>(args)...); }

	private:
		static void empty_dtor(void*){}

		void* m_handle = nullptr;
		R (*m_function)(void*, Args&&... args) = nullptr;
		void (*m_dtor)(void*) = empty_dtor;
	};


}

#endif