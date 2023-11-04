//@	{
//@	"target":{"name":"./dft_engine.o", "type":"object"},
//@	"dependencies":[{"ref":"OpenEXR", "origin":"pkg-config"}]
//@	}

#include "./dft_engine.hpp"

#include <mutex>

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