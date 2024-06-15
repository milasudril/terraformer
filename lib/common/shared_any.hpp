#ifndef TERRAFORMER_SHARED_ANY_HPP
#define TERRAFORMER_SHARED_ANY_HPP

#include "./any_pointer.hpp"

#include <cstddef>
#include <utility>
#include <compare>
#include <bit>
#include <cstdint>

namespace terraformer
{
	template<class T>
	concept controls_shared_resource = requires(T x)
	{
		{std::as_const(x).use_count()}->std::same_as<size_t>;
		{x.inc_usecount()};
		{x.dec_usecount()};
	};

	template<class LifetimeManager>
	class any_smart_pointer
	{
	public:
		using holder = LifetimeManager;

		any_smart_pointer() noexcept = default;

		template<class T, class ... Args>
		requires(!std::is_same_v<std::remove_const_t<T>, any_smart_pointer>)
		explicit any_smart_pointer(std::type_identity<T>, Args&&... args):
			m_holder{std::type_identity<T>{}, std::forward<Args>(args)...}
		{}

		~any_smart_pointer() noexcept
		{ reset(); }

		any_smart_pointer(any_smart_pointer&& other) noexcept:
			m_holder{std::exchange(other.m_holder, holder{})}
		{}

		any_smart_pointer& operator=(any_smart_pointer&& other) noexcept
		{
			reset();
			m_holder = std::exchange(other.m_holder, m_holder);
			return *this;
		}

		any_smart_pointer(any_smart_pointer const& other)
		requires controls_shared_resource<holder>:
			m_holder{other.m_holder}
		{ m_holder.inc_usecount(); }

		any_smart_pointer& operator=(any_smart_pointer const& other)
		requires controls_shared_resource<holder>
		{
			any_smart_pointer tmp{other};
			*this = std::move(tmp);
			return *this;
		}

		template<class T>
		T* get_if() const noexcept
		{ return m_holder.template get_if<T>(); }

		auto get() const noexcept
		{ return any_pointer{m_holder.pointer, m_holder.current_type}; }

		auto get_const() const noexcept
		{ return any_pointer{as_const(m_holder.pointer), m_holder.current_type}; }

		void reset() noexcept
		{
			m_holder.dec_usecount();
			m_holder = holder{};
		}

		std::strong_ordering operator<=>(any_smart_pointer const& other) const noexcept
		{ return m_holder <=> other.m_holder; }

		size_t use_count() const noexcept
		requires controls_shared_resource<holder>
		{ return m_holder.use_count(); }

		operator bool() const noexcept
		{ return m_holder.pointer != nullptr; }

		intptr_t object_id() const
		{ return std::bit_cast<intptr_t>(m_holder.pointer); }

	private:
		holder m_holder;
	};

	struct shared_any_holder
	{
		static void noop(void*){}

		shared_any_holder() = default;
		template<class T, class ... Args>
		explicit shared_any_holder(std::type_identity<T>, Args&&... args):
			pointer{new T(std::forward<Args>(args)...)},
			current_type{std::type_index{typeid(T)}},
			usecount{new size_t(1)},
			destroy{[](void* obj){ delete static_cast<T*>(obj);}}
		{}

		std::strong_ordering operator<=>(shared_any_holder const& other) const noexcept
		{ return std::compare_three_way{}(pointer, other.pointer); }

		void inc_usecount() noexcept
		{ ++(*usecount); }

		void dec_usecount() noexcept
		{
			if(usecount != nullptr)
			{
				--(*usecount);
				if(*usecount == 0)
				{
					destroy(pointer);
					delete usecount;
				}
			}
		}

		template<class T>
		T* get_if() const noexcept
		{
			if(current_type == std::type_index{typeid(T)})
			{ return static_cast<T*>(pointer); }
			return nullptr;
		}

		size_t use_count() const noexcept
		{ return usecount != nullptr? *usecount: static_cast<size_t>(0); }

		void* pointer = nullptr;
		std::type_index current_type = std::type_index{typeid(void)};
		size_t* usecount = nullptr;
		void (*destroy)(void*) = noop;
	};

	static_assert(controls_shared_resource<shared_any_holder>);

	using shared_any = any_smart_pointer<shared_any_holder>;
}
#endif