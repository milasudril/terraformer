//@	{"dependencies_extra": [{"ref":"./dft_engine.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_DFT_ENGINE_HPP
#define TERRAFORMER_DFT_ENGINE_HPP

#include <fftw3.h>
#include <complex>
#include <memory>
#include <type_traits>
#include <mutex>
#include <array>

namespace terraformer
{
	enum class dft_direction{forward = FFTW_FORWARD, backward = FFTW_BACKWARD};

	class dft_execution_plan
	{
	public:
		explicit dft_execution_plan(size_t size, dft_direction);

		dft_execution_plan() = default;

		void execute(std::complex<float> const* input_buffer, float* output_buffer) const
		{
			// NOTE: The plan is not configured for an inplace transform. Therefore, it is safe to do
			//       a const_cast on the input buffer.
			// NOTE: A fftwf_complex is ABI compatible with std::complex<float>
			auto input_buffer_ptr = reinterpret_cast<fftwf_complex*>(const_cast<std::complex<float>*>(input_buffer));
			auto output_buffer_ptr = reinterpret_cast<fftwf_complex*>(output_buffer);
			fftwf_execute_dft(m_plan.get(), input_buffer_ptr, output_buffer_ptr);
		}

		explicit operator bool() const { return static_cast<bool>(m_plan); }

	private:
		using plan_type = std::remove_pointer_t<fftwf_plan>;

		struct plan_deleter
		{
			void operator()(fftwf_plan plan)
			{ if(plan != nullptr) { fftwf_destroy_plan(plan); } }
		};

		std::unique_ptr<plan_type, plan_deleter> m_plan;
	};

	class dft_execution_plan_cache
	{
	public:
		dft_execution_plan const& get_plan(size_t buffer_size, dft_direction dir) const;

	private:
		static constexpr size_t cache_size = 16;
		mutable std::mutex m_access_mutex;
		mutable std::array<std::pair<size_t, dft_direction>, cache_size> m_transform_sizes{};
		struct plan_info
		{
			dft_execution_plan plan;
			size_t last_used{0};
		};
		mutable size_t m_counter{0};

		mutable std::array<plan_info, cache_size> m_plans;
	};

	dft_execution_plan const& get_plan(size_t buffer_size, dft_direction dir);
}

#endif