#ifndef TERRAFORMER_SHARED_ANY_HPP
#define TERRAFORMER_SHARED_ANY_HPP

#include <typeindex>
#include <cstddef>
#include <utility>
#include <compare>

namespace terraformer
{
	class shared_any
	{
	public:
		shared_any() noexcept = default;

		template<class T, class ... Args>
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