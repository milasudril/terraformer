//@	{"dependencies_extra":[{"ref":"./heightmap.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_HEIGHTMAP_HPP
#define TERRAFORMER_HEIGHTMAP_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/unique_resource.hpp"
#include "lib/generators/plain_generator/plain.hpp"
#include "lib/generators/rolling_hills_generator/rolling_hills_generator.hpp"
#include "lib/generators/domain/domain_size.hpp"
#include "lib/pixel_store/image.hpp"

namespace terraformer
{
	template<class T>
	concept heightmap_generator_source_descriptor = requires(domain_size_descriptor dom_size, T const& x)
	{
		{generate(dom_size, x)} -> std::same_as<grayscale_image>;
	};

	class heightmap_generator
	{
	public:
		template<heightmap_generator_source_descriptor Descriptor, class Form>
		explicit heightmap_generator(Descriptor&& params, Form& bind_to):
			m_resource{std::forward<Descriptor>(params)}
		{ bind(*static_cast<Descriptor*>(m_resource.get().get_pointer()), bind_to); }

		grayscale_image generate(domain_size_descriptor dom_size) const
		{
			auto const do_generate = m_resource.get().get_vtable().do_generate;
			auto const descriptor = m_resource.get().get_pointer();
			return do_generate(dom_size, descriptor);
		}

	private:
		struct vtable
		{
			template<heightmap_generator_source_descriptor Descriptor>
			explicit vtable(std::type_identity<Descriptor>):
				do_generate{[](domain_size_descriptor dom_size,  void const* descriptor){
					return generate(dom_size, *static_cast<Descriptor const*>(descriptor));
				}}
			{}

			grayscale_image (*do_generate)(domain_size_descriptor dom_size, void const* descriptor);
		};
		unique_resource<vtable> m_resource;
	};

	struct heightmap_generator_descriptor
	{
		plain_descriptor plain;
		rolling_hills_descriptor rolling_hills;
	};

	struct heightmap_descriptor
	{
		domain_size_descriptor domain_size;
		heightmap_generator_descriptor generators;
	};

	grayscale_image generate(heightmap_descriptor const& descriptor);
}

#endif