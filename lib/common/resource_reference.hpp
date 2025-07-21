#ifndef TERRAFORMER_RESOURCE_REFERENCE_HPP
#define TERRAFORMER_RESOURCE_REFERENCE_HPP

#include <type_traits>

namespace terraformer
{
	template<class Vtable, bool AddDtor>
	struct resource_reference_vtable;

	template<class Vtable>
	struct resource_reference_vtable<Vtable, false>:Vtable
	{
		template<class ... Tags>
		constexpr resource_reference_vtable(std::type_identity<Tags>...):
			Vtable{std::type_identity<Tags>{}...}
		{}
	};

	template<class Vtable>
	struct resource_reference_vtable<Vtable, true>:resource_reference_vtable<Vtable, false>
	{
		template<class ReferencedType, class ... Tags>
		constexpr resource_reference_vtable(std::type_identity<ReferencedType>, std::type_identity<Tags>...):
			resource_reference_vtable<Vtable,false>{
				std::type_identity<ReferencedType>{}, std::type_identity<Tags>{}...
			},
			destroy{
				[](void* object){
					delete static_cast<ReferencedType*>(object);
				}
			}
		{}

		void (*destroy)(void*);
	};


	template<class Vtable, bool AddDtor = false>
	class resource_reference
	{
	public:
		using vtable = resource_reference_vtable<Vtable, AddDtor>;

		resource_reference() = default;

		template<class ReferencedType, class... Tags>
		explicit resource_reference(std::reference_wrapper<ReferencedType> ref, std::type_identity<Tags>...):
			m_handle{&ref.get()},
			m_vtable_pointer{&s_vtable<ReferencedType, Tags...>}
		{}

		template<class ReferencedType, class... Tags>
		explicit resource_reference(ReferencedType* ref, std::type_identity<Tags>...):
			m_handle{ref},
			m_vtable_pointer{&s_vtable<ReferencedType, Tags...>}
		{}

		explicit resource_reference(void* handle, vtable const* vt):
			m_handle{handle},
			m_vtable_pointer{vt}
		{}

		template<class... Tags>
		static constexpr vtable s_vtable{std::type_identity<Tags>{}...};

		void* get_pointer() const
		{ return m_handle; }

		vtable const& get_vtable() const
		{ return *m_vtable_pointer; }


		operator bool() const
		{ return m_handle != nullptr; }

		explicit operator resource_reference<Vtable>() const
		{ return resource_reference<Vtable>{m_handle, m_vtable_pointer}; }

	private:
		void* m_handle{nullptr};
		vtable const* m_vtable_pointer{nullptr};
	};
}

#endif