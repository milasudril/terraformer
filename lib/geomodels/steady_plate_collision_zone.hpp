#ifndef TERRAFORMER_GEOMODELS_STEADYPLATECOLLISIONZONE_HPP
#define TERRAFORMER_GEOMODELS_STEADYPLATECOLLISIONZONE_HPP

#include "lib/curve_tool/ridge_curve.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/common/double_buffer.hpp"
#include "lib/filters/curve_rasterizer.hpp"
#include "lib/filters/diffuser.hpp"
#include "lib/filters/fill_from_points.hpp"
#include "lib/execution/thread_pool.hpp"

#include "lib/pixel_store/image_io.hpp"


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

	template<class Rng>
	void make_heightmap_2(double_buffer<terraformer::grayscale_image>& buffers,
		Rng&& rng,
		float pixel_size,
		steady_plate_collision_zone_descriptor const& heightmap_params)
	{
		auto const curve = generate(rng, pixel_size, heightmap_params.main_ridge);
		auto& current_buffer = buffers.back();
		auto const w = current_buffer.width();
		auto const h = current_buffer.height();

		for(uint32_t y = 0; y != h; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{
				assert(std::size(curve) == w);
				location const current_loc{
					pixel_size*static_cast<float>(x),
					pixel_size*static_cast<float>(y),
					0.0f
				};

				// NOTE: This works because curve is a function of x
				auto const side = current_loc[1] < curve[x][1]? -1.0f : 1.0f;

				auto const i = std::ranges::min_element(curve, [current_loc](auto a, auto b) {
					auto const loc_a = a - displacement{0.0f, 0.0f, a[2]};
					auto const loc_b = b - displacement{0.0f, 0.0f, b[2]};
					return distance(current_loc, loc_a) < distance(current_loc, loc_b);
				});
				auto const ridge_point = *i - displacement{0.0f, 0.0f, (*i)[2]};
				auto const distance_to_ridge = distance(current_loc, ridge_point);
				auto const z_boundary = side < 0.0f?
					heightmap_params.boundary.back_level:
					heightmap_params.boundary.front_level;
				auto const distance_to_boundary = side < 0.0f?
					current_loc[1]:
					pixel_size*static_cast<float>(h) - current_loc[1];
				auto const eta = distance_to_boundary/(distance_to_boundary + distance_to_ridge);
				auto const z_valley = z_boundary + eta*eta*(5120.0f - z_boundary);

				current_buffer(x, y) = z_valley;
			}

		}
		buffers.swap();
	}

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
		fill_from_points(main_ridge.pixels(), curve, [
				pixel_size,
				w = static_cast<float>(w)*pixel_size,
				h = static_cast<float>(h)*pixel_size,
				&boundary = heightmap_params.boundary
		](auto pixel_point, auto points){
			auto const r0 = location{0.0f, 0.0f, 0.0f} + (pixel_point - location{0.0f, 0.0f, 0.0f})*pixel_size;
			auto i = std::ranges::min_element(points, [r0](auto const a, auto const b) {
				auto const a_horz = a - displacement{0.0f, 0.0f, a[2]};
				auto const b_horz = b - displacement{0.0f, 0.0f, b[2]};
				return norm(r0 - a_horz) < norm(r0 - b_horz);
			});
			auto const O = *i;
			auto const P = r0 + displacement{0.0f, 0.0f, O[2]};
			return [w, h, & boundary](auto const ray) {
				auto const r = ray.target - ray.origin;
				auto const d_top = norm(r);
				auto const d_boundary = r[1] < 0.0f ? ray.target[1] : h - ray.target[1];
				auto const d_top_boundary = r[1] < 0.0f ? 16384.0f : h - 16384.0f;  // TODO: Test closest orthogonal point instead of average
				auto const z_boundary = r[1] < 0.0f ?  boundary.back_level : boundary.front_level;
				constexpr auto slope_factor = 2.0f;
				auto const xi = std::clamp(1.0f - d_top/(slope_factor*4096.0f), 0.0f, 1.0f);
				auto const eta = d_boundary/d_top_boundary;
				auto const z_valley = z_boundary + eta*eta*(5120.0f - z_boundary);
				return z_valley + (ray.origin[2] - z_valley)*xi*xi;
 			}(geosimd::ray{O, P});
		});
		store(main_ridge.pixels(), "test.exr");

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

		// TODO: Apply laplace only in proximity of the ridge. Would like to set curve elevation to
		//       a value relative to base level, rather than sea level.
		solve_bvp(buffers, terraformer::laplace_solver_params{
			.tolerance = 1.0e-3f * heightmap_params.main_ridge.start_location[2],
			.step_executor_factory = std::forward<DiffusionStepExecutorFactory>(exec_factory),
			.boundary = [
				values = main_ridge,
				front_back = heightmap_params.boundary
			](uint32_t x, uint32_t y) {
				if(y == 0)
				{
					return terraformer::dirichlet_boundary_pixel{
						.weight = 1.0f,
						.value = front_back.back_level
					};
				}

				if(y == values.height() - 1)
				{
					return terraformer::dirichlet_boundary_pixel{
						.weight=1.0f,
						.value=front_back.front_level
					};
				}
#if 0
				if(x == 0 || x == values.width())
				{
					auto const z_0 = front_back.back_level;
					auto const z_1 = front_back.front_level;
					auto const xi = static_cast<float>(y)/static_cast<float>(values.height());
					auto const z =  (z_1 - z_0)*xi*xi*(3.0f - 2.0f*xi) + z_0;

					return terraformer::dirichlet_boundary_pixel{
						.weight = 1.0f,
						.value = z
					};
				}
#endif

				auto const val = values(x, y);
				return val >= 0.5f ?
					terraformer::dirichlet_boundary_pixel{1.0f, val}:
					terraformer::dirichlet_boundary_pixel{0.0f, 0.0f};
			},
		});
	}
}

#endif