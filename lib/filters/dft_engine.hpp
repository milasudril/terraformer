//@	{"dependencies_extra": [{"ref":"./dft_engine.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_DFT_ENGINE_HPP
#define TERRAFORMER_DFT_ENGINE_HPP

#include "lib/common/span_2d.hpp"

#include <fftw3.h>
#include <complex>
#include <memory>
#include <type_traits>
#include <array>
#include <variant>

namespace terraformer
{
	enum class dft_direction{forward = FFTW_FORWARD, backward = FFTW_BACKWARD};

	class dft_execution_plan
	{
	public:
		explicit dft_execution_plan(size_t size, dft_direction dir);

		explicit dft_execution_plan(span_2d_extents size, dft_direction);

		dft_execution_plan() = default;

		void execute(std::complex<double> const* input_buffer, std::complex<double>* output_buffer) const
		{
			// NOTE: The plan is not configured for an inplace transform. Therefore, it is safe to do
			//       a const_cast on the input buffer.
			// NOTE: A fftwf_complex is ABI compatible with std::complex<double>
			auto input_buffer_ptr = reinterpret_cast<fftw_complex*>(const_cast<std::complex<double>*>(input_buffer));
			auto output_buffer_ptr = reinterpret_cast<fftw_complex*>(output_buffer);
			fftw_execute_dft(m_plan.get(), input_buffer_ptr, output_buffer_ptr);
		}

		explicit operator bool() const { return static_cast<bool>(m_plan); }

	private:
		using plan_type = std::remove_pointer_t<fftw_plan>;

		struct plan_deleter
		{
			void operator()(fftw_plan plan)
			{ if(plan != nullptr) { fftw_destroy_plan(plan); } }
		};

		std::unique_ptr<plan_type, plan_deleter> m_plan;
	};

	class dft_execution_plan_cache
	{
	public:
		using sizes = std::variant<size_t, span_2d_extents>;

		dft_execution_plan const& get_plan(sizes size, dft_direction dir) const;

	private:
		static constexpr size_t cache_size = 16;

		struct plan_info
		{
			dft_execution_plan plan;
			size_t last_used{0};
		};


		mutable size_t m_counter{0};
		mutable std::array<std::pair<sizes, dft_direction>, cache_size> m_transform_sizes{};
		mutable std::array<plan_info, cache_size> m_plans;
	};

	dft_execution_plan const& get_plan(dft_execution_plan_cache::sizes buffer_size, dft_direction dir);
}

#endif