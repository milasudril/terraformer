#ifndef TERRAFORMER_IMAGE_REGISTRY_VIEW_HPP
#define TERRAFORMER_IMAGE_REGISTRY_VIEW_HPP

#include "./span_2d.hpp"

#include <string_view>
#include <functional>
#include <stdexcept>

namespace terraformer
{
	class image_registry_view
	{
	public:
		template<class ImageRegistry>
		explicit image_registry_view(std::reference_wrapper<ImageRegistry> registry):
			m_handle{&registry.get()},
			m_get_image{[](void const* handle, std::u8string_view name) {
				auto& obj = *static_cast<ImageRegistry const*>(handle);
				auto i = obj.find(name);
				if(i == std::end(obj))
				{ throw std::runtime_error{"Key not found"}; }
				return i->second.pixels();
			}}
		{}

		span_2d<float const> get_image(std::u8string_view name) const
		{ return m_get_image(m_handle, name); }

	private:
		void const* m_handle;
		span_2d<float const> (*m_get_image)(void const*, std::u8string_view);
	};
}

#endif