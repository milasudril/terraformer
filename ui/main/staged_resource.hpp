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
		explicit staged_resource(FrontendResource&& resource):
			m_frontend_resource{std::move(resource)},
			m_must_update_backend_resource{true}
		{}

		template<class BackendResourceFactory>
		requires(
				std::is_base_of_v<global_instance_counter, BackendResourceFactory>
			&& resolve_overload<>(&BackendResourceFactory::get_global_id)
				== resolve_overload<>(&global_instance_counter::get_global_id)
		)
		BackendResource& get_backend_resource(BackendResourceFactory& backend)
		{
			if(!m_backend_resource.created_by_factory(backend.get_global_id())) [[unlikely]]
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
		BackendResource m_backend_resource;
		FrontendResource m_frontend_resource;
		bool m_must_update_backend_resource;
	};
}

#endif