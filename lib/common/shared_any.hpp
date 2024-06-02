#ifndef TERRAFORMER_SHARED_ANY_HPP
#define TERRAFORMER_SHARED_ANY_HPP

#include <typeindex>
#include <cstddef>
#include <utility>
#include <compare>
#include <bit>
#include <cstdint>

namespace terraformer
{
	template< class T >
	constexpr T const* as_const(T* t ) noexcept
	{ return t; }

	template<bool IsConst>
	class any_pointer
	{
	public:
		using pointer = std::conditional_t<IsConst, void const*, void*>;

		any_pointer() = default;

		explicit any_pointer(pointer ptr, std::type_index type):
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

	private:
		pointer m_pointer = nullptr;
		std::type_index m_type = std::type_index{typeid(void)};
	};

	using any_pointer_to_const = any_pointer<true>;

	any_pointer(void*, std::type_index) -> any_pointer<false>;

	any_pointer(void const*, std::type_index) -> any_pointer<true>;

	template<class T>
	any_pointer(T* ptr) -> any_pointer<std::is_const_v<T>>;

	class shared_any
	{
	public:
		shared_any() noexcept = default;

		template<class T, class ... Args>
		requires(!std::is_same_v<std::remove_const_t<T>, shared_any>)
		explicit shared_any(std::type_identity<T>, Args&&... args):
			m_holder{std::type_identity<T>{}, std::forward<Args>(args)...}
		{}

		~shared_any() noexcept
		{ reset(); }

		shared_any(shared_any&& other) noexcept:
			m_holder{std::exchange(other.m_holder, holder{})}
		{}

		shared_any& operator=(shared_any&& other) noexcept
		{
			reset();
			m_holder = std::exchange(other.m_holder, m_holder);
			return *this;
		}

		shared_any(shared_any const& other):
			m_holder{other.m_holder}
		{ m_holder.inc_usecount(); }

		shared_any& operator=(shared_any const& other)
		{
			shared_any tmp{other};
			*this = std::move(tmp);
			return *this;
		}

		template<class T>
		T* get_if() const noexcept
		{ return m_holder.get_if<T>(); }

		auto get() const noexcept
		{ return any_pointer{m_holder.pointer, m_holder.current_type}; }

		auto get_const() const noexcept
		{ return any_pointer{as_const(m_holder.pointer), m_holder.current_type}; }

		void reset() noexcept
		{
			m_holder.dec_usecount();
			m_holder = holder{};
		}

		std::strong_ordering operator<=>(shared_any const& other) const noexcept
		{ return m_holder <=> other.m_holder; }

		size_t use_count() const noexcept
		{ return m_holder.use_count(); }

		operator bool() const noexcept
		{ return m_holder.pointer != nullptr; }

		intptr_t object_id() const
		{ return std::bit_cast<intptr_t>(m_holder.pointer); }

	private:
		static void noop(void*){}

		struct holder
		{
			holder() = default;
			template<class T, class ... Args>
			explicit holder(std::type_identity<T>, Args&&... args):
				pointer{new T(std::forward<Args>(args)...)},
				current_type{std::type_index{typeid(T)}},
				usecount{new size_t(1)},
				destroy{[](void* obj){ delete static_cast<T*>(obj);}}
			{}

			std::strong_ordering operator<=>(holder const& other) const noexcept
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

		holder m_holder;
	};
}
#endif