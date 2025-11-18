//@	{"dependencies_extra":[{"ref":"./filter_utils.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FILTER_UTILS_HPP
#define TERRAFORMER_FILTER_UTILS_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/utils.hpp"
#include "lib/execution/batch_result.hpp"
#include "lib/math_utils/computation_context.hpp"
#include <complex>

namespace terraformer
{
	void make_filter_input(
		scanline_processing_job_info const& jobinfo,
		span_2d<std::complex<float>> output,
		span_2d<float const> input
	);

	void make_filter_output(
		scanline_processing_job_info const& jobinfo,
		span_2d<float> output,
		span_2d<std::complex<float> const> input
	);

	class filter_2d_job
	{
	public:
		explicit filter_2d_job(batch_result<void> br, unique_handle temp_buffer):
			m_br{std::move(br)}, m_temp_buffer{std::move(temp_buffer)}
		{}
		void wait()
		{ m_br.wait(); }

	private:
		batch_result<void> m_br;
		unique_handle m_temp_buffer;
	};

	filter_2d_job apply_filter(
		span_2d<float const> input,
		span_2d<float> filtered_output,
		computation_context& comp_ctxt,
		span_2d<float const> filter_mask
	);
}

#endif