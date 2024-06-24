#ifndef TERRAFORMER_ANY_SMART_POINTER_HPP
#define TERRAFORMER_ANY_SMART_POINTER_HPP

#include "./any_pointer.hpp"

#include <utility>
#include <cstddef>
#include <cstdint>
#include <bit>

namespace terraformer
{
	template<class T>
	concept controls_shared_resource = requires(T x)
	{
		{std::as_const(x).use_count()}->std::same_as<size_t>;
		{x.inc_usecount()};
		{x.dec_usecount()};
	};
	
	template<class T, class LifetimeManager>
	class smart_pointer;

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
			if constexpr(controls_shared_resource<LifetimeManager>)
			{ m_holder.dec_usecount(); }
			else
			{ m_holder.delete_resource(); }

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
	
	template<class T, class LifetimeManager>
	class smart_pointer:private any_smart_pointer<LifetimeManager>
	{
	public:
		using base = any_smart_pointer<LifetimeManager>;
		
		smart_pointer() = default;

		template<class Head, class ... Args>
		requires(!std::is_same_v<std::remove_const_t<Head>, smart_pointer>)
		explicit smart_pointer(std::type_identity<T>, Args&&... args):
			base{std::type_identity<T>{}, std::forward<Args>(args)...}
		{}
		
		T* get() const
		{ return base::template get_if<T>(); }
		
		T const* get_const() const
		{ return base::template get_if<T const>(); }
		
		using base::reset;
		using base::operator<=>;
		using base::operator bool;
		using base::use_count;
		using base::object_id;
	};
}
#endif
