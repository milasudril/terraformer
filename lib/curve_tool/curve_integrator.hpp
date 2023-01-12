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

	struct particle_state
	{
		location r;
		displacement v;
		displacement v_ext;
	};

	auto guess_next_state(particle_state const& state_prev,
		displacement v_ext,
		float inertia,
		float scaling_factor)
	{
		// This expression follows from using the trapetzoid rule, assuming v_ext is independent
		// of the current system state.
		auto const v_guess = ((2.0f*inertia - 1.0f)/(1.0f + 2.0f*inertia)) * state_prev.v
			+ (v_ext + state_prev.v_ext)/(1.0f + 2.0f*inertia);
		auto const r_guess =  state_prev.r + scaling_factor*(v_guess + state_prev.v)/2.0f;

		return particle_state{r_guess, v_guess, v_ext};
	}

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
			displacement* v_ext_prev){
			displacement const v_ext = curve_params.velocity_model(old_states);
			auto const inertia = curve_params.inertia;
			auto const scaling_factor = curve_params.scaling_factor;

			auto const& state_prev = old_states.back();

			particle_state const ps_in{
				.r = state_prev.locations(k),
				.v = state_prev.velocities(k),
				.v_ext = v_ext_prev[k]
			};

			auto const state_guess = guess_next_state(ps_in,
				v_ext,
				inertia,
				scaling_factor);

			displacement const dr = state_guess.r - ps_in.r;

			// We will not use v_ext_prev when correcting for bad angles. Thus, it is ok
			// to save the value already at this point
			v_ext_prev[k] = state_guess.v_ext;

			auto const point_count = std::size(old_states);
			auto const idist_prev = state_prev.integ_distances(k);
			auto const iheading_prev = state_prev.integ_headings(k);

			if(point_count < 2)
			{
				auto const integ_distance = idist_prev + norm(dr);
				auto const integ_heading = iheading_prev
					+ angular_difference(direction{dr}, direction{ps_in.v});

				return particle{
					.r = state_guess.r,
					.v = state_guess.v,
					.integ_distance = integ_distance,
					.integ_heading = integ_heading
				};
			}

			auto const r_prev_prev = old_states[point_count - 2].locations(k);

			direction const dir_in{ps_in.r - r_prev_prev};

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

			auto const r = ps_in.r + d*dir_corr;
			auto const v = 2.0f*(r - ps_in.r)/scaling_factor  - ps_in.v;
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
