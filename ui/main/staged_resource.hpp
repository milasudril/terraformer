#ifndef TERRAFORMER_UI_MAIN_STAGED_RESOURCE_HPP
#define TERRAFORMER_UI_MAIN_STAGED_RESOURCE_HPP

#include "lib/common/global_instance_counter.hpp"
#include "lib/common/utils.hpp"

namespace terraformer::ui::main
{
	template<class BackendResource, class FrontendResource>
	class staged_resource
	{
	public:
		staged_resource() = default;

		explicit staged_resource(FrontendResource&& resource):
			m_frontend_resource{std::move(resource)},
			m_must_update_backend_resource{true}
		{}

		staged_resource& operator=(FrontendResource&& resource)
		{
			m_frontend_resource = std::move(resource);
			m_must_update_backend_resource = true;
			return *this;
		}

		operator FrontendResource const&() const
		{ return m_frontend_resource; }

		FrontendResource const& frontend_resource() const
		{ return m_frontend_resource; }

		template<class GraphicsBackend>
		BackendResource& get_backend_resource(GraphicsBackend& backend) const
		{
			if(!m_backend_resource.belongs_to_backend(backend.get_global_id())) [[unlikely]]
			{
				m_backend_resource = backend.create(std::type_identity<BackendResource>{}, m_frontend_resource);
				m_must_update_backend_resource = false;
				return m_backend_resource;
			}

			if(m_must_update_backend_resource) [[unlikely]]
			{
				m_backend_resource.upload(m_frontend_resource);
				m_must_update_backend_resource = false;
			}
			return m_backend_resource;
		}

	private:
		mutable BackendResource m_backend_resource;
		FrontendResource m_frontend_resource;
		mutable bool m_must_update_backend_resource;
	};
}

#endif