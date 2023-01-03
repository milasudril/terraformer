//@	{"target":{"name":"main.o"}}

#include "lib/curve_integrator.hpp"

#include <random>
#include <pcg-cpp/include/pcg_random.hpp>

using random_generator = pcg_engines::oneseq_dxsm_128_64;

namespace
{
	using terraformer::particle_system;
	using terraformer::location;
	using terraformer::displacement;
	using terraformer::particle;
	using terraformer::curve_parameters;

	struct curve:private std::vector<location>
	{
		using value_type = location;

		using std::vector<location>::vector;
		using std::vector<location>::begin;
		using std::vector<location>::end;
		using std::vector<location>::push_back;
		using std::vector<location>::reserve;
		using std::vector<location>::size;
		using std::vector<location>::front;
		using std::vector<location>::back;
	};

	auto get_curves(std::vector<particle_system> const& state_history)
	{
		std::vector<curve> ret;
		if(std::size(state_history) == 0)
		{ return ret; }

		ret.resize(std::size(state_history.front()));

		for(size_t k = 0; k != std::size(state_history); ++k)
		{
			auto const locations = state_history[k].locations();
			for(size_t l = 0; l != std::size(locations); ++l)
			{
				ret[l].push_back(locations[l]);
			}
		}

		return ret;
	}

	auto random_vector(random_generator& rng)
	{
		auto U = std::uniform_real_distribution{-1.0f, 1.0f};
		while(true)
		{
			if(auto const ret = displacement{U(rng), U(rng), 0.0f}; norm_squared(ret) < 1.0f)
			{ return ret; }
		}
	}
}

int main()
{
	random_generator rng{};
	size_t const num_particles = 1;

	particle_system ps{
		num_particles,
		[](size_t k, size_t N) {
			auto const y = static_cast<float>(k) - 0.5f*static_cast<float>(N) + 0.5f;
			return particle{
				.r = location{0.0f, 0.5f*y, 0.0f},
				.v = displacement{1.0f/16.0f, 0.0f, 0.0f},
				.integ_distance{0.0f},
				.integ_heading{geosimd::turn_angle{0}}
			};
		},
		num_particles
	};

	curve_parameters const params{
		.velocity_model = [&rng](std::span<particle_system const>){
			displacement const drift{1.0f/16.0f, 0.0f, 0.0f};

			return drift + random_vector(rng);
		},
		.inertia = 2.0f,
		.scaling_factor = 1.0f/32.0f
	};

	auto const state_history = generate_curves(std::move(ps),
		[](particle_system const& current_state) -> bool {
			auto const i_max = std::ranges::max_element(current_state.integ_distances());
			return *i_max > 1.0f;
		}, params);


	auto const curves = get_curves(state_history);

	std::ranges::for_each(curves[0], [](auto const item) {
		auto const [x, y, _] = item;
		printf("%.8e %.8e\n", x, y);
	});
}
