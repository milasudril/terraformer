#ifndef TERRAFORMER_UNIQUE_ANY_HPP
#define TERRAFORMER_UNIQUE_ANY_HPP

#include "./any_smart_pointer.hpp"

#include <compare>

namespace terraformer
{
	template<bool IsConst>
	struct unique_any_holder
	{
		template<class T>
		using pointer_type = std::conditional_t<IsConst, T const*, T*>;

		static void noop(pointer_type<void>){}

		unique_any_holder() = default;
		template<class T, class ... Args>
		explicit unique_any_holder(std::type_identity<T>, Args&&... args):
			pointer{new T(std::forward<Args>(args)...)},
			current_type{std::type_index{typeid(T)}},
			destroy{[](pointer_type<void> obj){ delete static_cast<pointer_type<T>>(obj);}}
		{}

		std::strong_ordering operator<=>(unique_any_holder const& other) const noexcept
		{ return std::compare_three_way{}(pointer, other.pointer); }

		void delete_resource() noexcept
		{ destroy(pointer); }

		template<class T>
		pointer_type<T> get_if() const noexcept
		{
			if(current_type == std::type_index{typeid(T)})
			{ return static_cast<pointer_type<T>>(pointer); }
			return nullptr;
		}

		pointer_type<void> pointer = nullptr;
		std::type_index current_type = std::type_index{typeid(void)};
		void (*destroy)(pointer_type<void>) = noop;
	};

	static_assert(!controls_shared_resource<unique_any_holder<false>>);
	static_assert(!controls_shared_resource<unique_any_holder<true>>);

	using unique_any = any_smart_pointer<unique_any_holder<false>>;
	using unique_const_any = any_smart_pointer<unique_any_holder<true>>;
}
#endif