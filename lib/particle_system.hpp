#ifndef TERRAFORMER_LIB_PARTICLE_SYSTEM_HPP
#define TERRAFORMER_LIB_PARTICLE_SYSTEM_HPP

#include "./spaces.hpp"

#include <span>
#include <memory>
#include <ranges>

namespace terraformer
{
	struct particle
	{
		location r;
		displacement v;
		float integ_distance;
		geosimd::turn_angle integ_heading;
	};

	template<class T>
	concept particle_generator = requires(T x)
	{
		{x(size_t{})} -> std::same_as<particle>;
	};

	class particle_system
	{
	public:
		template<particle_generator Generator>
		explicit particle_system(size_t size, Generator gen):m_size{size},
			m_locations{std::make_unique_for_overwrite<location[]>(size)},
			m_velocities{std::make_unique_for_overwrite<displacement[]>(size)},
			m_integ_distances{std::make_unique_for_overwrite<float[]>(size)},
			m_integ_headings{std::make_unique_for_overwrite<geosimd::turn_angle[]>(size)}
		{
			for(size_t k = 0; k != size; ++k)
			{
				assign(k, gen(k));
			}
		}



		size_t size() const
		{ return m_size; }

		auto& assign(size_t k, particle const& particle)
		{
			m_locations[k] = particle.r;
			m_velocities[k] = particle.v;
			m_integ_distances[k] = particle.integ_distance;
			m_integ_headings[k] = particle.integ_heading;

			return *this;
		}

		auto get_particle(size_t k) const
		{
			return particle{
				.r = m_locations[k],
				.v = m_velocities[k],
				.integ_distance = m_integ_distances[k],
				.integ_heading = m_integ_headings[k]
			};
		}



		auto locations() const
		{ return std::span{m_locations.get(), m_size}; }

		auto locations()
		{ return std::span{m_locations.get(), m_size}; }

		auto locations(size_t k) const
		{ return m_locations[k]; }

		auto& locations(size_t k)
		{ return m_locations[k]; }



		auto velocities() const
		{ return std::span{m_velocities.get(), m_size}; }

		auto velocities()
		{ return std::span{m_velocities.get(), m_size}; }

		auto velocities(size_t k) const
		{ return m_velocities[k]; }

		auto& velocities(size_t k)
		{ return m_velocities[k]; }



		auto integ_distances() const
		{ return std::span{m_integ_distances.get(), m_size}; }

		auto integ_distances()
		{ return std::span{m_integ_distances.get(), m_size}; }

		auto integ_distances(size_t k) const
		{ return m_integ_distances[k]; }

		auto& integ_distances(size_t k)
		{ return m_integ_distances[k]; }



		auto integ_headings() const
		{ return std::span{m_integ_headings.get(), m_size}; }

		auto integ_headings()
		{ return std::span{m_integ_headings.get(), m_size}; }

		auto integ_headings(size_t k) const
		{ return m_integ_headings[k]; }

		auto& integ_headings(size_t k)
		{ return m_integ_headings[k]; }

	private:
		size_t m_size;
		std::unique_ptr<location[]> m_locations;
		std::unique_ptr<displacement[]> m_velocities;
		std::unique_ptr<float[]> m_integ_distances;
		std::unique_ptr<geosimd::turn_angle[]> m_integ_headings;
	};
};

#endif