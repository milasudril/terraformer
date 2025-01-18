#ifndef TERRAFORMER_UNIQUE_RESOURCE_HPP
#define TERRAFORMER_UNIQUE_RESOURCE_HPP

#include <utility>
#include <type_traits>
#include <memory>

namespace terraformer
{
	template<class Vtable>
	class resource_reference
	{
	public:
		struct vtable:Vtable
		{
			template<class ReferencedType>
			constexpr vtable(std::type_identity<ReferencedType>):
				Vtable{std::type_identity<ReferencedType>{}},
				destroy{
					[](void* object){
						delete static_cast<ReferencedType*>(object);
					}
				}
			{}

			void (*destroy)(void*);
		};

		resource_reference() = default;

		template<class ReferencedType>
		explicit resource_reference(std::reference_wrapper<ReferencedType> ref):
			m_handle{&ref},
			m_vtable_pointer{&s_vtable<ReferencedType>}
		{}

		explicit resource_reference(void* handle, vtable const* vt):
			m_handle{handle},
			m_vtable_pointer{vt}
		{}

		template<class ReferencedType>
		static constexpr vtable s_vtable{std::type_identity<ReferencedType>{}};

		void* get_pointer() const
		{ return m_handle; }

		Vtable const& get_vtable() const
		{ return *m_vtable_pointer; }

		operator bool() const
		{ return m_handle != nullptr; }

	private:
		void* m_handle{nullptr};
		vtable const* m_vtable_pointer{nullptr};
	};

	template<class Vtable>
	class unique_resource
	{
	public:
		unique_resource() = default;

		template<class OwnedType>
		explicit unique_resource(std::unique_ptr<OwnedType> obj):
			m_handle{obj.release()},
			m_vtable_pointer{&resource_reference<Vtable>::template s_vtable<std::remove_cvref_t<OwnedType>>}
		{ }

		template<class OwnedType>
		requires(!std::is_same_v<std::remove_cvref_t<OwnedType>, unique_resource>)
		explicit unique_resource(OwnedType&& obj):
			m_handle{new std::remove_cvref_t<OwnedType>(std::forward<OwnedType>(obj))},
			m_vtable_pointer{&resource_reference<Vtable>::template s_vtable<std::remove_cvref_t<OwnedType>>}
		{ }

		template<class OwnedType, class... Args>
		explicit unique_resource(std::in_place_type_t<OwnedType>, Args&&... args):
			m_handle{new OwnedType(std::forward<Args>(args)...)},
			m_vtable_pointer{&resource_reference<Vtable>::template s_vtable<std::remove_cvref_t<OwnedType>>}
		{}

		~unique_resource()
		{ reset(); }

		unique_resource(unique_resource&& other):
			m_handle{std::exchange(other.m_handle, nullptr)},
			m_vtable_pointer{std::exchange(other.m_vtable_pointer, nullptr)}
		{ }

		unique_resource& operator=(unique_resource&& other)
		{
			m_handle = std::exchange(other.m_handle, m_handle);
			m_vtable_pointer = std::exchange(other.m_vtable_pointer, m_vtable_pointer);
			other.reset();
			return *this;
		}

		unique_resource(unique_resource const& other) = delete;

		unique_resource& operator=(unique_resource const&) = delete;

		resource_reference<Vtable> get() const
		{ return resource_reference<Vtable>{m_handle, m_vtable_pointer}; }

		operator bool() const
		{ return m_handle != nullptr; }

		void reset()
		{
			if(m_handle == nullptr)
			{ return; }

			m_vtable_pointer->destroy(m_handle);
			m_handle = nullptr;
			m_vtable_pointer = nullptr;
		}

	private:
		void* m_handle{nullptr};
		typename resource_reference<Vtable>::vtable const* m_vtable_pointer{nullptr};
	};
}

#endif