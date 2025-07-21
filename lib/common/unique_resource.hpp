#ifndef TERRAFORMER_UNIQUE_RESOURCE_HPP
#define TERRAFORMER_UNIQUE_RESOURCE_HPP

#include "./resource_reference.hpp"

#include <utility>
#include <memory>

namespace terraformer
{
	template<class Vtable>
	class unique_resource
	{
	public:
		unique_resource() = default;

		template<class OwnedType>
		explicit unique_resource(std::unique_ptr<OwnedType> obj) noexcept:
			m_handle{obj.release()}
		{ }

		template<class OwnedType>
		requires(!std::is_same_v<std::remove_cvref_t<OwnedType>, unique_resource>)
		explicit unique_resource(OwnedType&& obj):
			m_handle{new std::remove_cvref_t<OwnedType>(std::forward<OwnedType>(obj))}
		{ }

		template<class OwnedType, class... Args>
		explicit unique_resource(std::in_place_type_t<OwnedType>, Args&&... args):
			m_handle{new OwnedType(std::forward<Args>(args)...)}
		{}

		~unique_resource()
		{ reset(); }

		unique_resource(unique_resource&& other) noexcept:
			m_handle{std::exchange(other.m_handle, resource_reference<Vtable, true>{})}
		{ }

		unique_resource& operator=(unique_resource&& other) noexcept
		{
			m_handle = std::exchange(other.m_handle, m_handle);
			other.reset();
			return *this;
		}

		unique_resource(unique_resource const& other) = delete;

		unique_resource& operator=(unique_resource const&) = delete;

		resource_reference<Vtable> get() const
		{ return resource_reference<Vtable>{m_handle}; }

		operator bool() const
		{ return static_cast<bool>(m_handle); }

		void reset()
		{
			if(!m_handle)
			{ return; }

			m_handle.get_vtable().destroy(m_handle.get_pointer());
			m_handle = resource_reference<Vtable, true>{};
		}

	private:
		resource_reference<Vtable, true> m_handle;
	};
}

#endif