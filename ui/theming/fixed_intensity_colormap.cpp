//@	{"target": {"name":"fixed_intensity_colormap.o"}}

#include "./fixed_intensity_colormap.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"

namespace
{
	constexpr std::array<terraformer::rgba_pixel, 6> generate_lut()
	{
		using fic = terraformer::ui::theming::fixed_intensity_colormap;

		std::array<terraformer::rgba_pixel, 6> ret{
			fic::normalize(terraformer::rgba_pixel{1.0f, 0.0f, 0.0f, 0.0f}),
			terraformer::rgba_pixel{},
			fic::normalize(terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 0.0f}),
			fic::max_blue_compensate_with_other(1.0f),
			fic::max_blue_compensate_with_other(0.5f),
			fic::max_blue_compensate_with_other(0.0f)
		};
		ret[1] = 0.5f*(ret[0] + ret[2]);

		for(size_t k = 0; k != std::size(ret); ++k)
		{
			ret[k] = terraformer::rgba_pixel{ret[k].red(), ret[k].green(), ret[k].blue()};
		}

		return ret;
	}

	constexpr auto lut = generate_lut();
}

std::array<terraformer::rgba_pixel, 6> const& terraformer::ui::theming::fixed_intensity_colormap::get_lut()
{
	return lut;
}

terraformer::rgba_pixel terraformer::ui::theming::fixed_intensity_colormap::operator()(float t) const
{
	auto const& lut = get_lut();
	return interp(get_lut(), static_cast<float>(std::size(lut))*t, wrap_around_at_boundary{});
}

#if 0
	std::array<color, 7> lut{
		color{
			.r = 0.9563970166379805f,
			.g = 0.0f,
			.b = 0.0f,
			.padding = 0.0f
		},
		color{
			.r = 0.9563970166379805f/2.0f,
			.g = 0.5426432291666666f/2.0f,
			.b = 0.0f,
			.padding = 0.0f
		},
		color{
			.r = 0.0f,
			.g = 0.5426432291666666f,
			.b = 0.0f,
			.padding = 0.0f
		},
		color{
			.r = 0.0f,
			.g = 0.4821851618939881f,
			.b = 0.9985332401164976f,
			.padding = 0.0f
		},
		color{
			.r = 0.30761808854465983f,
			.g = 0.30761808854465983f,
			.b = 0.9990263637498f,
			.padding = 0.0f
		},
		color{
			.r = 0.8497686747665654f,
			.g = 0.0f,
			.b = 0.9992107520530993f,
			.padding = 0.0f
		},
		color{
			.r = 0.9563970166379805f,
			.g = 0.0f,
			.b = 0.0f,
			.padding = 0.0f
		}
	};

	std::array<std::pair<color, size_t>, 12> palette{};
	for(size_t k = 0; k != std::size(palette); ++k)
	{
		auto const x_out_start = static_cast<float>(k)/std::size(palette);
		auto const x_stretched = x_out_start*(std::size(lut) - 1);

		auto const index_start = static_cast<size_t>(x_stretched);
		auto const index_end = index_start + 1;

		auto const prev = lut[index_start];
		auto const current = lut[index_end];
		auto const t = x_stretched - static_cast<float>(index_start);

		palette[k] = std::pair{interp(prev, current, t), k};
	}

	std::array<std::pair<color, size_t>, 12> palette_dark{};
	for(size_t k = 0; k != std::size(palette); ++k)
	{
		palette_dark[k] = std::pair{color{0.5f*palette[k].first.r, 0.5f*palette[k].first.g, 0.5f*palette[k].first.b, 0.5f*palette[k].first.padding}, k};
	}

	std::array<std::pair<color, size_t>, 12> palette_light{};
	for(size_t k = 0; k != std::size(palette); ++k)
	{
		palette_light[k] = std::pair{
			color{
				0.75f*palette[k].first.r + 0.25f,
				0.75f*palette[k].first.g + 0.25f,
				0.75f*palette[k].first.b + 0.25f,
				0.75f*palette[k].first.padding + 0.25f
			},
			k
		};
	}
	#endif