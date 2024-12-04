#ifndef TERRAFORMER_UI_MAIN_STAGED_RESOURCE_HPP
#define TERRAFORMER_UI_MAIN_STAGED_RESOURCE_HPP

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
		BackendResource& get_backend_resource(BackendResourceFactory backend)
		{
			if(!m_backend_resource.is_compatible(backend)) [[unlikely]]
			{
				m_backend_resource = value_of(backend)
					.create(std::type_identity<BackendResource>{}, m_frontend_resource);
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