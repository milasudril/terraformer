//@	{"dependencies_extra":[{"ref":"./heightmap.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_HEIGHTMAP_HPP
#define TERRAFORMER_HEIGHTMAP_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/unique_resource.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/filters/heightmap_to_mesh.hpp"
#include "lib/filters/modulator/modulator.hpp"
#include "lib/generators/plain_generator/plain.hpp"
#include "lib/generators/rolling_hills_generator/rolling_hills_generator.hpp"
#include "lib/generators/domain/domain_size.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/array_classes/single_array.hpp"

#include <map>
#include <string>

namespace terraformer
{
	template<class T>
	concept heightmap_generator_source_descriptor = requires(
		 T& x,
		 domain_size_descriptor dom_size,
		 descriptor_editor_ref editor
	)
	{
		{std::as_const(x).generate_heightmap(dom_size)} -> std::same_as<grayscale_image>;
		{x.bind(editor)} -> std::same_as<void>;
	};

	class heightmap_generator
	{
	public:
		template<heightmap_generator_source_descriptor Descriptor>
		explicit heightmap_generator(Descriptor&& params) requires
		(!std::is_same_v<heightmap_generator, std::remove_cvref_t<Descriptor>>):
			m_resource{std::forward<Descriptor>(params)}
		{ }

		heightmap_generator(heightmap_generator const& other):
			m_resource{
				other.m_resource.get().get_vtable().do_clone(other.m_resource.get().get_pointer())
			}
		{}
		~heightmap_generator() = default;

		heightmap_generator(heightmap_generator&&) = default;
		heightmap_generator& operator=(heightmap_generator&&) = default;
		heightmap_generator& operator=(heightmap_generator const&) = default;

		grayscale_image generate_heightmap(domain_size_descriptor dom_size) const
		{
			auto const do_generate = m_resource.get().get_vtable().do_generate;
			auto const descriptor = m_resource.get().get_pointer();
			return do_generate(dom_size, descriptor);
		}

		void bind(descriptor_editor_ref editor)
		{
			auto const do_bind = m_resource.get().get_vtable().do_bind;
			auto const descriptor = m_resource.get().get_pointer();
			do_bind(descriptor, editor);
		}

	private:
		struct vtable
		{
			template<heightmap_generator_source_descriptor Descriptor>
			constexpr explicit vtable(std::type_identity<Descriptor>):
				do_generate{[](domain_size_descriptor dom_size,  void const* descriptor){
					return static_cast<Descriptor const*>(descriptor)->generate_heightmap(dom_size);
				}},
				do_bind{[](void* descriptor, descriptor_editor_ref editor){
					return static_cast<Descriptor*>(descriptor)->bind(editor);
				}},
				do_clone{[](void const* descriptor){
					return unique_resource<vtable>{*static_cast<Descriptor const*>(descriptor)};
				}}
			{}

			grayscale_image (*do_generate)(domain_size_descriptor, void const* descriptor);
			void (*do_bind)(void*, descriptor_editor_ref);
			unique_resource<vtable> (*do_clone)(void const*);
		};

		unique_resource<vtable> m_resource;
	};

	struct heightmap_generator_channel_strip_descriptor
	{
		std::u8string input;
		filters::modulator_descriptor modulation;
		float gain = 1.0f;

		void bind(descriptor_editor_ref editor);
	};

	struct heightmap_descriptor
	{
		domain_size_descriptor domain_size;
		std::map<std::u8string, heightmap_generator, std::less<>> generators{
			{u8"Plain", heightmap_generator{plain_descriptor{}}},
			{u8"Rolling hills", heightmap_generator{rolling_hills_descriptor{}}}
		};

		std::array<heightmap_generator_channel_strip_descriptor, 2> channel_strips{
			heightmap_generator_channel_strip_descriptor{
				.input = u8"Plain",
				.modulation = filters::modulator_descriptor{},
				.gain = 1.0f
			},
			heightmap_generator_channel_strip_descriptor{
				.input = u8"Rolling hills",
				.modulation = filters::modulator_descriptor{
					.modulator = u8"Plain",
					.modulator_exponent = 2.0f,
					.modulation_depth = 1.0f
				},
				.gain = 1.0f
			}
		};

		void bind(descriptor_editor_ref editor);
	};

	grayscale_image generate(heightmap_descriptor const& descriptor);
}

#endif