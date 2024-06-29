#ifndef TERRAFORMER_SHARED_ANY_HPP
#define TERRAFORMER_SHARED_ANY_HPP

#include "./any_smart_pointer.hpp"

#include <compare>
#include <cstdio>

namespace terraformer
{
	template<bool IsConst>
	struct shared_any_holder
	{
		static void noop(void*){}

		static constexpr auto const_flag = IsConst;

		template<class Dummy = void>
		requires(IsConst)
		shared_any_holder(shared_any_holder<false> const& other):
			pointer{other.pointer},
			current_type{other.current_type},
			usecount{other.usecount},
			destroy{other.destroy}
		{ }

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
		requires((std::is_const_v<T> == IsConst) || !IsConst)
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

	static_assert(controls_shared_resource<shared_any_holder<false>>);
	static_assert(controls_shared_resource<shared_any_holder<true>>);

	using shared_any = any_smart_pointer<shared_any_holder<false>>;
	using shared_const_any = any_smart_pointer<shared_any_holder<true>>;
}
#endif