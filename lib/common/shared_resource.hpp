#ifndef TERRAFORMER_SHARED_RESOURCE_HPP
#define TERRAFORMER_SHARED_RESOURCE_HPP

#include "./resource_reference.hpp"

#include <utility>
#include <memory>
#include <atomic>

namespace terraformer
{
	template<class Vtable>
	class shared_resource
	{
	public:
		shared_resource() = default;

		template<class OwnedType>
		explicit shared_resource(std::unique_ptr<OwnedType> obj):
			m_handle{obj.release()},
			m_refcount{new std::atomic<size_t>{1}}
		{ }

		template<class OwnedType>
		requires(!std::is_same_v<std::remove_cvref_t<OwnedType>, shared_resource>)
		explicit shared_resource(OwnedType&& obj):
			m_handle{new std::remove_cvref_t<OwnedType>(std::forward<OwnedType>(obj))},
			m_refcount{new std::atomic<size_t>{1}}
		{ }

		template<class OwnedType, class... Args>
		explicit shared_resource(std::in_place_type_t<OwnedType>, Args&&... args):
			m_handle{new OwnedType(std::forward<Args>(args)...)},
			m_refcount{new std::atomic<size_t>{1}}
		{}

		~shared_resource()
		{ reset(); }

		shared_resource(shared_resource&& other) noexcept:
			m_handle{std::exchange(other.m_handle, resource_reference<Vtable, true>{})},
			m_refcount{std::exchange(other.m_refcount, nullptr)}
		{ }

		shared_resource& operator=(shared_resource&& other) noexcept
		{
			m_handle = std::exchange(other.m_handle, m_handle);
			m_refcount = std::exchange(other.m_refcount, m_refcount);
			other.reset();
			return *this;
		}

		shared_resource(shared_resource const& other) noexcept:
			m_handle{other.m_handle},
			m_refcount{other.m_refcount}
		{ ++(*m_refcount); }

		shared_resource& operator=(shared_resource const& other)
		{
			shared_resource tmp{other};
			*this = std::move(tmp);
			return *this;
		}

		resource_reference<Vtable> get() const
		{ return resource_reference<Vtable>{m_handle}; }

		operator bool() const
		{ return static_cast<bool>(m_handle); }

		void reset()
		{
			if(!m_handle)
			{ return; }

			if(m_refcount->fetch_sub(1) - 1 == 0)
			{
				m_handle.get_vtable().destroy(m_handle.get_pointer());
				delete m_refcount;
			}

			m_handle = resource_reference<Vtable, true>{};
		}

	private:
		resource_reference<Vtable, true> m_handle;
		std::atomic<size_t>* m_refcount{nullptr};
	};
}

#endif