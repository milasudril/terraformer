//@	{"dependencies_extra":[{"ref":"./ridge_tree.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/curve_tools/displace.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/span_2d.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_tree_branch_description
	{
		ridge_tree_branch_displacement_description displacement_profile;
		ridge_tree_branch_growth_description growth_params;
	};

	struct ridge_tree_xy_description
	{
		location root_location;
		direction trunk_direction;
		std::vector<ridge_tree_branch_description> curve_levels;
	};

	struct ridge_tree_branch_elevation_profile
	{
		ridge_tree_ridge_elevation_profile_description base_elevation;
		ridge_tree_elevation_modulation_description modulation;
	};

	struct wave_descriptor
	{
		float amplitude;
		float wavelength;
		float hf_rolloff = 2.0f;
	};

	/**
	 * Computes a suitable step or pixel size given the scale properties of a wave
	 *
	 * Assume a filter for which the frequency response is
	 * \f[
	 * |H(\omega)| = \frac{1}{\sqrt{1 + \left(\frac{\omega^2}{\omega_0^2}\right)^n}}
	 * \f]
	 *
	 * If the input signal is white noise with amplitude \(A\), the strength of the output
	 * signal is
	 * \f[
	 * |Y(\omega)| = \frac{A}{\sqrt{1 + \left(\frac{f}{f_0}\right)^{2n}}}
	 * \f]
	 *
	 * The goal is to determine a sample rate \f$f_s\f$ such that frequencies with amplitude greater
	 * than one can be represented. Notice that the step size is \f$1/f_s\f$. For the to be satisfied,
	 * all frequencies above the Nyquist frequency \f$1f_n = f_s/2\f$ should be less than some
	 * threshold \f$A_0\f$, that is
	 * \f[
	 * \frac{A}{\sqrt{1 + \left(\frac{f_s}{2 f_0}\right)^{2n}}} < A_0
	 * \f]
	 *
	 * Which gives
	 *
	 * \f[
	 * \sqrt{1 + \left(\frac{f_s}{2 f_0}\right)^{2n}} > \frac{A}{A_0}
	 * \f]
	 *
	 * This requires that $A > 1$. Since \f$f_s\f$ must be greater than
	 * \f[
	 * 2 f_0 \left(\frac{A}{A_0}^2 - 1\right)^{\frac{1}{2n}}
	 * \f]
	 * it is sufficient to pick
	 * \f[
	 * f_s = 2 f_0 \frac{A}{A_0}^{\frac{1}{n}}
	 * \f]
	 * which is the limit as \f$\frac{A}{A_0}\to\infty\f$.
	 *
	 * For the purpose of this function, \f$A_0\f$ is set to 1 m.
	 *
	 * \todo Move this function to mathutils or something so it can be reused
	 *
	 */
	inline float get_pixel_size(wave_descriptor const& wave)
	{
		auto const amplitude = wave.amplitude;
		auto const wavelength = wave.wavelength;
		auto const hf_rolloff = wave.hf_rolloff;
		auto const a_0 = 1.0f;
		auto const dx = wavelength*std::pow(amplitude/a_0, -1.0f/hf_rolloff)/2.0f;

		printf("%.8g\n", dx);

		return 0.5f*std::min(dx, 0.5f*wavelength);
	}

	class ridge_tree
	{
	public:
	  explicit ridge_tree(ridge_tree_xy_description const& description, random_generator& rng);

		operator span<ridge_tree_trunk const>() const
		{ return m_value; }

		auto element_indices(size_t skip = 0) const
		{ return m_value.element_indices(skip); }

		auto size() const
		{ return std::size(m_value);  }

		auto begin() const
		{ return std::begin(m_value); }

		auto end() const
		{ return std::end(m_value); }

		auto data() const
		{ return std::data(m_value); }

		auto const& operator[](array_index<ridge_tree_trunk> index) const
		{ return m_value[index]; }

		void update_elevations(
			elevation initial_elevation,
			std::span<ridge_tree_branch_elevation_profile const> elevation_profiles,
			random_generator& rng
		);

	private:
		single_array<ridge_tree_trunk> m_value;
	};

	inline ridge_tree generate(
		ridge_tree_xy_description const& description,
		random_generator& rng
	)
	{ return ridge_tree{description, rng}; }

	struct ridge_tree_peak_radius_range
	{
		float min;
		float max;
	};

	struct ridge_tree_branch_render_description
	{
		ridge_tree_slope_angle_range half_peak_angle;
		ridge_tree_slope_angle_range foot_angle;
		ridge_tree_peak_radius_range peak_radius;
	};

	struct ridge_tree_render_description
	{
		std::vector<ridge_tree_branch_render_description> curve_levels;
	};

	void render(ridge_tree const& tree,
		span_2d<float> output,
		ridge_tree_render_description const& params,
		float pixel_size);
}

#endif