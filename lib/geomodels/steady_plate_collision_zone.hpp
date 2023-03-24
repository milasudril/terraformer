#ifndef TERRAFORMER_GEOMODELS_STEADYPLATECOLLISIONZONE_HPP
#define TERRAFORMER_GEOMODELS_STEADYPLATECOLLISIONZONE_HPP

#include "lib/curve_tool/ridge_curve.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/common/double_buffer.hpp"
#include "lib/filters/curve_rasterizer.hpp"
#include "lib/filters/diffuser.hpp"
#include "lib/execution/thread_pool.hpp"

namespace terraformer
{
	struct domain_boundary_conditions
	{
		float front_level;
		float back_level;
	};

	struct steady_plate_collision_zone_descriptor
	{
		domain_boundary_conditions boundary;
		main_ridge_params main_ridge;
	};

	template<class Rng, class DiffusionStepExecutorFactory>
	void make_heightmap(double_buffer<terraformer::grayscale_image>& buffers,
		Rng&& rng,
		DiffusionStepExecutorFactory&& exec_factory,
		float pixel_size,
		steady_plate_collision_zone_descriptor const& heightmap_params)
	{
		auto const curve = generate(rng, pixel_size, heightmap_params.main_ridge);
		auto const w = buffers.front().width();
		auto const h = buffers.front().height();

		terraformer::grayscale_image main_ridge{w, h};
		draw(main_ridge.pixels(), curve, terraformer::line_segment_draw_params{
			.value = 1.0f,
			.scale = pixel_size
		});

		generate(buffers.back().pixels(), [
				y_0 = heightmap_params.main_ridge.start_location[1]/pixel_size,
				z_0 = heightmap_params.main_ridge.start_location[2],
				h = static_cast<float>(h),
				boundary = heightmap_params.boundary
			](uint32_t, uint32_t y) {
			auto const y_val = static_cast<float>(y);

			auto const t1 = y_val/y_0;
			auto const t2 = (y_val - y_0)/(h - y_0);

			auto const z1 = std::lerp(boundary.back_level, z_0, t1);
			auto const z2 = std::lerp(z_0, boundary.front_level, t2);

			return t1 < 1.0f ? z1 : z2;
		});
		buffers.swap();

		solve_bvp(buffers, terraformer::laplace_solver_params{
			.tolerance = 1.0e-6f * heightmap_params.main_ridge.start_location[2],
			.step_executor_factory = std::forward<DiffusionStepExecutorFactory>(exec_factory),
			.boundary = [
				values = main_ridge,
				front_back = heightmap_params.boundary
			](uint32_t x, uint32_t y) {
				if(y == 0)
				{
					return terraformer::dirichlet_boundary_pixel{
						.weight=1.0f,
						.value=front_back.back_level
					};
				}

				if(y == values.height() - 1)
				{
					return terraformer::dirichlet_boundary_pixel{
						.weight=1.0f,
						.value=front_back.front_level
					};
				}

				auto const val = values(x, y);
				return val >= 0.5f ?
					terraformer::dirichlet_boundary_pixel{1.0f, val}:
					terraformer::dirichlet_boundary_pixel{0.0f, 0.0f};
			},
		});
	}
}

#endif