#ifndef TERRAFORMER_UNIQUE_RESOURCE_HPP
#define TERRAFORMER_UNIQUE_RESOURCE_HPP

#include <utility>
#include <type_traits>

namespace terraformer
{
template<class Vtable>
class unique_resource
{
	public:
		unique_resource() = default;

		template<class OwnedType, class... Args>
		explicit unique_resource(std::in_place_type_t<OwnedType>, Args&&... args):
			m_vtable_pointer{&s_vtable<OwnedType>},
			m_handle{new OwnedType(std::forward<Args>(args)...)}
		{}

		~unique_resource()
		{
			if(m_handle != nullptr)
			{ m_vtable_pointer->destroy(m_handle); }
		}

		unique_resource(unique_resource&& other):
			m_vtable_pointer{std::exchange(other.m_vtable_pointer, nullptr)}.
			m_handle{std::exchange(other.m_handle, nullptr)}
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

		void* get_pointer() const
		{ return m_handle; }

		Vtable const& get_vtable() const
		{ return *m_vtable_pointer; }

		operator bool() const
		{ return m_handle != nullptr; }

		void reset()
		{
			m_vtable_pointer->destroy(m_handle);
			m_handle = nullptr;
			m_vtable_pointer = nullptr;
		}

	private:
		struct vtable:Vtable
		{
			template<class OwnedType>
			constexpr vtable(std::type_identity<OwnedType>):
				Vtable{std::type_identity<OwnedType>{}},
				destroy{
					[](void* object){
						delete static_cast<OwnedType*>(object);
					}
				}
			{}

			void (*destroy)(void*);
		};

		template<class OwnedType>
		static constexpr vtable s_vtable{std::type_identity<OwnedType>{}};

		vtable const* m_vtable_pointer{nullptr};
		void* m_handle{nullptr};
	};
}

#endif