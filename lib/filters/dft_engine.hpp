//@	{"dependencies_extra": [{"ref":"./dft_engine.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_DFT_ENGINE_HPP
#define TERRAFORMER_DFT_ENGINE_HPP

#include <fftw3.h>
#include <complex>
#include <memory>
#include <type_traits>

namespace terraformer
{
	enum class dft_direction{forward = FFTW_FORWARD, backward = FFTW_BACKWARD};

	class dft_execution_plan
	{
	public:
		explicit dft_execution_plan(size_t size, dft_direction);

		void execute(std::complex<float> const* input_buffer, float* output_buffer)
		{
			// NOTE: The plan is not configured for an inplace transform. Therefore, it is safe to do
			//       a const_cast on the input buffer.
			// NOTE: A fftwf_complex is ABI compatible with std::complex<float>
			auto input_buffer_ptr = reinterpret_cast<fftwf_complex*>(const_cast<std::complex<float>*>(input_buffer));
			auto output_buffer_ptr = reinterpret_cast<fftwf_complex*>(output_buffer);
			fftwf_execute_dft(m_plan.get(), input_buffer_ptr, output_buffer_ptr);
		}

	private:
		using plan_type = std::remove_pointer_t<fftwf_plan>;

		struct plan_deleter
		{
			void operator()(fftwf_plan plan)
			{ if(plan != nullptr) { fftwf_destroy_plan(plan); } }
		};

		std::unique_ptr<plan_type, plan_deleter> m_plan;
	};
}

#endif