#ifndef TERRAFORMER_INTENSITY_HPP
#define TERRAFORMER_INTENSITY_HPP

#include "./rgba_pixel.hpp"

#include <algorithm>

namespace terraformer
{
	template<float RedWeight, float GreenWeight, float BlueWeight>
	class intensity
	{
	public:
		static constexpr rgba_pixel::storage_type weights{RedWeight, GreenWeight, BlueWeight, 0.0f};

		static constexpr intensity max_value{weights[0] + weights[1] + weights[2]};

		constexpr explicit intensity(float value): m_value{value}{}

		constexpr explicit intensity(rgba_pixel v)
		{
			auto const tmp = v.value()*weights;
			m_value = tmp[0] + tmp[1] + tmp[2];
		}

		constexpr float value() const
		{ return m_value; }

		constexpr auto operator<=>(intensity const&) const = default;

	private:
		float m_value;
	};

	template<float RedWeight, float GreenWeight, float BlueWeight>
	constexpr float operator-(
		intensity<RedWeight, GreenWeight, BlueWeight> i1,
		intensity<RedWeight, GreenWeight, BlueWeight> i2
	)
	{ return i1.value() - i2.value(); }

	template<float RedWeight, float GreenWeight, float BlueWeight>
	constexpr float operator/(
		intensity<RedWeight, GreenWeight, BlueWeight> i1,
		intensity<RedWeight, GreenWeight, BlueWeight> i2
	)
	{ return i1.value() / i2.value(); }

	template<float RedWeight, float GreenWeight, float BlueWeight>
	constexpr rgba_pixel brighten(
		rgba_pixel input,
		intensity<RedWeight, GreenWeight, BlueWeight> target_intensity
	)
	{
		using intensity_type = intensity<RedWeight, GreenWeight, BlueWeight>;
		intensity_type const input_intensity{input};
		auto const intensity_white = intensity_type::max_value;
		target_intensity = std::clamp(target_intensity, input_intensity, intensity_white);
		auto const t  = (target_intensity - input_intensity)/(intensity_white - input_intensity);

		return t*rgba_pixel{1.0f, 1.0f, 1.0f, 0.0f} + (1.0f - t)*input;
	}

	template<float RedWeight, float GreenWeight, float BlueWeight>
	constexpr rgba_pixel normalize(
		rgba_pixel input,
		intensity<RedWeight, GreenWeight, BlueWeight> target_intensity
	)
	{
		using intensity_type = intensity<RedWeight, GreenWeight, BlueWeight>;
		auto const tmp = input*(target_intensity/intensity_type{input});
		auto const maxval = max_color_value(tmp);
		if(maxval > 1.0f)
		{ return brighten(tmp/maxval, target_intensity); }
		return tmp;
	}

	template<float RedWeight, float GreenWeight, float BlueWeight>
	inline auto to_string(intensity<RedWeight, GreenWeight, BlueWeight> x)
	{ return std::to_string(x.value()); }

	using perceptual_color_intensity = terraformer::intensity<0.5673828125f, 1.0f, 0.060546875f>;
}
#endif