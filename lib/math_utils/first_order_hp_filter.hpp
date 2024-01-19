#ifndef TERRAFORMER_FIRST_ORDER_HP_FILTER_HPP
#define TERRAFORMER_FIRST_ORDER_HP_FILTER_HPP

namespace terraformer
{
	struct first_order_hp_filter_description
	{
		float cutoff_freq;
		float initial_value;
		float initial_input;
	};

	class first_order_hp_filter
	{
	public:
 		constexpr explicit first_order_hp_filter(first_order_hp_filter_description const& params, float dt):
			m_dt{dt},
			m_y{params.initial_value},
			m_x{params.initial_input},
			m_cutoff_freq{params.cutoff_freq}
		{}

		constexpr float operator()(float x)
		{
			// ẏ + ωy = ẋ
			// ẏ = -ωy + ẋ
			// y[n] = y[n - 1] + c(-ωy[n] + 2ẋ - ωy[n - 1])/2

			auto const dt = m_dt;
			auto const ω = m_cutoff_freq;
			auto const y_prev = m_y;
			auto const x_prev = m_x;
			auto const x_dot = (x - x_prev)/dt;
			auto const y = (2.0f*dt*x_dot - y_prev*(dt*ω - 2.0f))/(dt*ω + 2.0f);
			m_y = y;
			m_x = x;
			return y;
		}

	private:
		float m_dt;
		float m_y;
		float m_x;
		float m_cutoff_freq;
	};
}

#endif