#ifndef TERRAFORMER_ANY_POINTER_HPP
#define TERRAFORMER_ANY_POINTER_HPP

#include <type_traits>
#include <typeindex>

namespace terraformer
{
	template< class T >
	constexpr T const* as_const(T* t ) noexcept
	{ return t; }

	template<bool IsConst>
	class any_pointer
	{
	public:
		using pointer_type = std::conditional_t<IsConst, void const*, void*>;

		any_pointer() = default;

		template<class Dummy = void>
		requires(IsConst)
		any_pointer(any_pointer<false> other):
			m_pointer{other.pointer()},
			m_type{other.type()}
		{}

		explicit any_pointer(pointer_type ptr, std::type_index type):
			m_pointer{ptr},
			m_type{type}
		{}

		template<class T>
		explicit any_pointer(T* ptr):
			m_pointer{ptr},
			m_type{std::type_index{typeid(T)}}
		{}

		template<class T>
		operator T*() const noexcept
		{ return std::type_index{typeid(T)} == m_type? static_cast<T*>(m_pointer) : nullptr; }

		template<class T>
		T* get_if() const noexcept
		{
			if(m_type == std::type_index{typeid(T)})
			{ return static_cast<T*>(m_pointer); }
			return nullptr;
		}

		operator bool() const noexcept
		{ return m_pointer != nullptr; }

		auto operator<=>(any_pointer const&) const = default;

		auto pointer() const
		{ return m_pointer; }

		auto type() const
		{ return m_type; }

	private:
		pointer_type m_pointer = nullptr;
		std::type_index m_type = std::type_index{typeid(void)};
	};

	using any_pointer_to_const = any_pointer<true>;

	using any_pointer_to_mutable = any_pointer<false>;

	any_pointer(void*, std::type_index) -> any_pointer<false>;

	any_pointer(void const*, std::type_index) -> any_pointer<true>;

	template<class T>
	any_pointer(T* ptr) -> any_pointer<std::is_const_v<T>>;
}

#endif