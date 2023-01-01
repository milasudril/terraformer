#ifndef TERRAFORMER_LIB_CURVE_INTEGRATOR_HPP
#define TERRAFORMER_LIB_CURVE_INTEGRATOR_HPP

#include "./particle_system.hpp"

#include <span>
#include <vector>

namespace terraformer
{
	template<class T>
	concept curve_velocity_model = requires(T x)
	{
		{x(std::span<particle_system>{})} -> std::same_as<displacement>;
	};

	template<class T>
	concept curve_integrator_stop_condition = requires(T x, particle_system const& p)
	{
		{x(p)} -> std::same_as<bool>;
	};

	template<curve_velocity_model V>
	struct curve_parameters
	{
		V velocity_model;
		float inertia;
		float scaling_factor;
	};

	template<curve_velocity_model V>
	auto compute_next_state(particle_system const& initial_state,
		std::span<particle_system const> old_states,
		curve_parameters<V> const& curve_params,
		displacement* v_ext_prev)
	{
		auto const particle_count = std::size(initial_state);
		particle_system ret{particle_count, [](size_t k,
			auto old_states,
			auto curve_params,
			auto v_ext_prev){
			auto const v_ext = curve_params.velocity_model(old_states);
			auto const& state_prev = old_states.back();
			auto const v_prev = state_prev.velocities(k);
			auto const I = curve_params.inertia;

			// This expression follows from using the trapetzoid rule, assuming v_ext is independent
			// of the current system state.
			auto const v_guess = ((2.0f*I - 1.0f)/(1.0f + 2.0f*I)) * v_prev
				+ (v_ext + v_ext_prev[k])/(1.0f + 2.0f*I);
			auto const c = curve_params.scaling_factor;
			auto const r_prev = state_prev.locations(k);
			auto const r_guess =  r_prev + c*(v_guess + v_prev)/2.0f;

			auto const dr = r_guess - r_prev;

			// We will not use v_ext_prev when correcting for bad angles. Thus, it is ok
			// to save the value already at this point
			v_ext_prev[k] = v_ext;

			auto const point_count = std::size(old_states);
			auto const idist_prev = state_prev.integ_distances(k);
			auto const iheading_prev = state_prev.integ_headings(k);

			if(point_count < 2)
			{
				auto const integ_distance = idist_prev + norm(dr);
				auto const integ_heading = iheading_prev
					+ angular_difference(direction{dr}, direction{geom_space::x{}});

				return particle{
					.r = r_guess,
					.v = v_guess,
					.integ_distance = integ_distance,
					.integ_heading = integ_heading
				};
			}

			auto const r_prev_prev = old_states[point_count - 2].locations(k);

			direction const dir_in{r_prev - r_prev_prev};

			auto const dtheta_in = angular_difference(direction{dr}, dir_in);
			geosimd::turn_angle const max_turn_angle{0x4000'0000};
			auto dtheta = clamp(dtheta_in, -max_turn_angle, max_turn_angle);

			geosimd::turn_angle const max_heading{0x4000'0000};
			if(dtheta + iheading_prev > max_heading)
			{ dtheta = max_heading - iheading_prev; }
			else
			if(dtheta + iheading_prev < -max_heading)
			{ dtheta = -max_heading - iheading_prev; }

			auto const theta_ref = angular_difference(dir_in, direction{geom_space::x{}});
			auto const cs = cossin(theta_ref + dtheta);

			auto const dir_corr = direction{cs, geom_space::z{}}; // As if rotated ccw around z-axis

			// Use the magnitude of the projection of dr on to the corrected direction. This
			// simulates a collision that preserves the tangential velocity is kept, while resets
			// the normal velocity
			auto const d = std::abs(inner_product(dir_corr, dr));

			auto const r = r_prev + d*dir_corr;
			auto const v = 2.0f*(r - r_prev)/c  - v_prev;
			auto const integ_distance = idist_prev + d;
			auto const integ_heading = iheading_prev + dtheta;

			return particle{
				.r = r,
				.v = v,
				.integ_distance = integ_distance,
				.integ_heading = integ_heading
			};
		}, old_states, curve_params, v_ext_prev};
		return ret;
	}


	template<curve_integrator_stop_condition S, curve_velocity_model V>
	auto generate_curves(particle_system&& initial_state,
		S const& stop_condition,
		curve_parameters<V> const& curve_params)
	{
		std::vector<particle_system> ret;
		auto const particle_count = std::size(initial_state);
		auto v_ext = std::make_unique_for_overwrite<displacement[]>(particle_count);
		for(size_t k = 0; k != particle_count; ++k)
		{
			v_ext[k] = curve_params.velocity_model(ret);
		}

		ret.push_back(std::move(initial_state));
		while(!stop_condition(ret.back()))
		{
			ret.push_back(compute_next_state(initial_state, ret, curve_params, v_ext.get()));
		}

		return ret;
	}

}

#endif
