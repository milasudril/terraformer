#ifndef TERRAFORMER_SECOND_ORDER_LP_FILTER_HPP
#define TERRAFORMER_SECOND_ORDER_LP_FILTER_HPP

namespace terraformer
{
	struct second_order_lp_filter_description
	{
		float damping;
		float cutoff_freq;
		float initial_value;
		float initial_derivative;
		float initial_input;
	};

	class second_order_lp_filter
	{
	public:
 		explicit second_order_lp_filter(second_order_lp_filter_description const& params, float dt):
			m_dt{dt},
			m_y{params.initial_value},
			m_v{params.initial_derivative},
			m_x_prev{params.initial_input},
			m_damping{params.damping},
			m_cutoff_freq{params.cutoff_freq}
		{}

		float operator()(float x)
		{
			// ÿ + 2ζωẏ + ω²y = xω²
			//
			// Let v = ẏ, and ÿ = ÿ
			//
			// v̇ + 2ζωv + yω² = xω²
			// ẏ = v
			//
			// v̇ = -(2ζωv + yω²) + xω²
			// ẏ = v
			//
			// v[n] = v[n - 1] + %Delta t*(-(2*ζ*ω*v[n] + y[n]*ω²) + x[n]*ω² -(2*ζ*ω*v[n - 1] + y[n - 1]*ω²) + x[n - 1]*ω² )/2
			// y[n] = y[n - 1] + %Delta t*(v[n] + v[n - 1])/2

			auto const dt = m_dt;
			auto const v_prev = m_v;
			auto const y_prev = m_y;
			auto const x_prev = m_x_prev;
			auto const ζ = m_damping;
			auto const ω = m_cutoff_freq;

			auto const denom = dt*dt*ω*ω + 4.0f*dt*ω*ζ + 4.0f;
			auto const v = (2.0f*(x - 2.0f*y_prev + x_prev)*dt*ω*ω - v_prev*(dt*dt*ω*ω + 4.0f*dt*ω*ζ - 4.0f))/denom;
			auto const y = (dt*dt*ω*ω*x + dt*dt*ω*ω*x_prev + 4.0f*dt*v_prev - y_prev*(dt*dt*ω*ω - 4.0f*dt*ω*ζ - 4.0f))/denom;

			m_v = v;
			m_y = y;
			m_x_prev = x;
			return y;
		}

	private:
		float m_dt;
		float m_y;
		float m_v;
		float m_x_prev;
		float m_damping;
		float m_cutoff_freq;
	};
}

#endif