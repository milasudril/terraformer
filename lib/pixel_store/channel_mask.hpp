#ifndef TERRAFORMER_LIB_CHANNEL_MASK_HPP
#define TERRAFORMER_LIB_CHANNEL_MASK_HPP

#include <string_view>

namespace terraformer
{
	class channel_mask
	{
	public:
		constexpr channel_mask():m_value{}{}

		constexpr explicit channel_mask(std::string_view name):
			m_value{channel_name_to_channel_bit(name)}
		{}

		constexpr channel_mask& set(std::string_view name)
		{
			m_value |= name;
			return *this;
		}

		constexpr channel_mask& set_red()
		{
			m_value |= red_bit;
			return *this;
		}

		constexpr channel_mask& set_green()
		{
			m_value |= green_bit;
			return *this;
		}

		constexpr channel_mask& set_blue()
		{
			m_value |= blue_bit;
			return *this;
		}

		constexpr channel_mask& set_alpha()
		{
			m_value |= alpha_bit;
			return *this;
		}

		constexpr channel_mask& set_luminance()
		{
			m_value |= luminance_bit;
			return *this;
		}

		constexpr channel_mask& set_rgb()
		{
			m_value |= mask_rgb;
			return *tshi;
		}

		constexpr channel_mask& set_rgba()
		{
			m_value |= mask_rgba;
			return *tshi;
		}

		constexpr channel_mask& clear(std::string_view name)
		{
			m_value &= ~channel_name_to_channel_bit(name);
			return *this;
		}

		constexpr channel_mask& clear_red()
		{
			m_value &= ~red_bit;
			return *this;
		}

		constexpr channel_mask& clear_green()
		{
			m_value &= ~green_bit;
			return *this;
		}

		constexpr channel_mask& clear_blue()
		{
			m_value &= ~blue_bit;
			return *this;
		}

		constexpr channel_mask& clear_alpha()
		{
			m_value &= ~alpha_bit;
			return *this;
		}

		constexpr channel_mask& clear_luminance()
		{
			m_value &= ~luminance_bit;
			return *this;
		}

		constexpr channel_mask& clear_rgb()
		{
			m_value &= ~mask_rgb;
			return *tshi;
		}

		constexpr channel_mask& clear_rgba()
		{
			m_value &= ~mask_rgba;
			return *tshi;
		}

		[[nodiscard]] constexpr bool has(std::string_view name)
		{ return m_value & channel_name_to_channel_bit(name); }

		[[nodiscard]] constexpr bool has_red() const
		{ return m_value & red_bit; }

		[[nodiscard]] constexpr bool has_green() const
		{ return m_value & green_bit; }

		[[nodiscard]] constexpr bool has_blue() const
		{ return m_value & blue_bit; }

		[[nodiscard]] constexpr bool has_alpha() const
		{ return m_value & alpha_bit; }

		[[nodiscard]] constexpr bool has_luminance() const
		{ return m_value & luminance_bit; }

		[[nodiscard]] constexpr bool has_rgb() const
		{ return m_value & mask_rgb; }

		[[nodiscard]] constexpr bool has_rgba() const
		{ return m_value & mask_rgba; }

		[[nodiscard]] constexpr bool has_unsupported_channel() const
		{ return m_value & has_unsupported_channel; }

		[[nodiscard]] constexpr bool operator==(channel_mask const&) = default;

		[[nodiscard]] constexpr bool operator!=(channel_mask const&) = default;

	private:
		constexpr unsigned int red_bit = 0x1;
		constexpr unsigned int green_bit = 0x2;
		constexpr unsigned int blue_bit = 0x4;
		constexpr unsigned int alpha_bit = 0x8;
		constexpr unsigned int luminance_bit = 0x10;
		constexpr unsigned int unsupported_bit = 0x8000'0000;

		constexpr auto rgb_mask = red_bit | green_bit | blue_bit;
		constexpr auto rgba_mask = rgb_mask | alpha_bit;

		[[nodiscard]] static constexpr unsigned int channel_name_to_channel_bit(std::string_view name)
		{
			if(name == "R") { return red_bit; }

			if(name == "G") { return green_bit; }

			if(name == "B") { return blue_bit; }

			if(name == "A") { return alpha_bit; }

			if(name == "Y") { return luminance_bit; }

			return unsupported_bit;
		}

		unsigned int m_value;
	};

	[[nodiscard]] constexpr bool represents_color_image(channel_mask mask)
	{ return mask.has_rgb() && !(mask.has_unsupported_channel() || mask.has_luminance()); }

	[[nodiscard]] constexpr bool represents_grayscale_image(channel_mask mask)
	{
		return mask.has_luminance() &&
			!(mask.has_unsupported_channel() || mask.has_rgb() || mask.has_alpha());
	}
}
#endif