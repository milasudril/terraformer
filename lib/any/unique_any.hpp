#ifndef TERRAFORMER_UNIQUE_ANY_HPP
#define TERRAFORMER_UNIQUE_ANY_HPP

#include "./any_smart_pointer.hpp"

#include <compare>

namespace terraformer
{
	struct unique_any_holder
	{
		static void noop(void*){}

		unique_any_holder() = default;
		template<class T, class ... Args>
		explicit unique_any_holder(std::type_identity<T>, Args&&... args):
			pointer{new T(std::forward<Args>(args)...)},
			current_type{std::type_index{typeid(T)}},
			destroy{[](void* obj){ delete static_cast<T*>(obj);}}
		{}

		std::strong_ordering operator<=>(unique_any_holder const& other) const noexcept
		{ return std::compare_three_way{}(pointer, other.pointer); }

		void delete_resource() noexcept
		{ destroy(pointer); }

		template<class T>
		T* get_if() const noexcept
		{
			if(current_type == std::type_index{typeid(T)})
			{ return static_cast<T*>(pointer); }
			return nullptr;
		}

		void* pointer = nullptr;
		std::type_index current_type = std::type_index{typeid(void)};
		void (*destroy)(void*) = noop;
	};

	static_assert(!controls_shared_resource<unique_any_holder>);

	using unique_any = any_smart_pointer<unique_any_holder>;
}
#endif