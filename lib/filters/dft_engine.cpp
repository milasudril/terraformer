//@	{
//@	"target":{"name":"./dft_engine.o", "type":"object"},
//@	"dependencies":[{"ref":"fftw3f", "origin":"pkg-config"}]
//@	}

#include "./dft_engine.hpp"

#include <algorithm>

namespace
{
	constinit std::mutex FftwPlanMutex;
}

terraformer::dft_execution_plan::dft_execution_plan(size_t size, dft_direction dir)
{
	auto input_buff  = std::make_unique<std::complex<float>[]>(size);
	auto output_buff = std::make_unique<std::complex<float>[]>(size);
	std::fill_n(input_buff.get(), size, 0);
	auto input_buff_ptr  = reinterpret_cast<fftwf_complex*>(input_buff.get());
	auto output_buff_ptr = reinterpret_cast<fftwf_complex*>(output_buff.get());
	std::lock_guard fftw_plan_lock{FftwPlanMutex};
	m_plan = std::unique_ptr<plan_type, plan_deleter>{fftwf_plan_dft_1d(static_cast<int>(size),
	                                                                  input_buff_ptr,
	                                                                  output_buff_ptr,
	                                                                  static_cast<int>(dir),
	                                                                  FFTW_MEASURE)};
}

terraformer::dft_execution_plan const&
terraformer::dft_execution_plan_cache::get_plan(size_t buffer_size, dft_direction dir) const

{
	std::lock_guard lock{m_access_mutex};
	auto const i = std::ranges::find(m_transform_sizes, std::pair{buffer_size, dir});
	if(i != std::end(m_transform_sizes)) [[likely]]
	{
		auto const index = i - std::begin(m_transform_sizes);
		auto& plan_info = m_plans[index];
		if(!plan_info.plan)
		{ plan_info.plan = dft_execution_plan{buffer_size, dir}; }

		plan_info.last_used = m_counter;
		++m_counter;
		return plan_info.plan;
	}

	auto const reject = std::ranges::min_element(m_plans, [](auto const& a, auto const& b) {
		return a.last_used < b.last_used;
	});

	auto const reject_index = reject - std::begin(m_plans);
	reject->plan = dft_execution_plan{buffer_size, dir};
	reject->last_used = m_counter;
	++m_counter;
	m_transform_sizes[reject_index] = std::pair{buffer_size, dir};
	return reject->plan;
}

namespace
{
	constinit terraformer::dft_execution_plan_cache dft_execution_plans;
}

terraformer::dft_execution_plan const& terraformer::get_plan(size_t buffer_size, dft_direction dir)
{
	return dft_execution_plans.get_plan(buffer_size, dir);
}