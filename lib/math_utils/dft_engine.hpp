//@	{"dependencies_extra": [{"ref":"./dft_engine.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_DFT_ENGINE_HPP
#define TERRAFORMER_DFT_ENGINE_HPP

#include "lib/common/span_2d.hpp"
#include "lib/execution/signaling_counter.hpp"

#include <fftw3.h>
#include <complex>
#include <memory>
#include <mutex>
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

		explicit dft_execution_plan(span_2d_extents size, dft_direction dir);

		dft_execution_plan() = default;

		void execute(std::complex<float> const* input_buffer, std::complex<float>* output_buffer) const
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

		std::shared_ptr<plan_type> m_plan;
	};

	class dft_execution_plan_cache
	{
	public:
		using sizes = std::variant<size_t, span_2d_extents>;

		dft_execution_plan get_plan(sizes size, dft_direction dir);

	private:
		static constexpr size_t cache_size = 16;

		struct plan_info
		{
			dft_execution_plan plan;
			size_t last_used{0};
		};

		size_t m_counter{0};
		std::array<std::pair<sizes, dft_direction>, cache_size> m_transform_sizes{};
		std::array<plan_info, cache_size> m_plans;
	};

	dft_execution_plan get_plan(dft_execution_plan_cache::sizes buffer_size, dft_direction dir);

	class dft_engine
	{
	public:
		template<class TaskRunner>
		static void enable_multithreading(TaskRunner& task_runner)
		{
			fftwf_init_threads();
			fftw_plan_with_nthreads(static_cast<int>(task_runner.max_concurrency()));
			fftw_threads_set_callback(
				[](void *(*work)(char *), char *jobdata, size_t elsize, int njobs, void* task_runner) {
					*m_status = signaling_counter{static_cast<size_t>(njobs)};
					auto& obj = *static_cast<TaskRunner*>(task_runner);
					for (int i = 0; i < njobs; ++i)
					{
						obj.submit(
							[work, jobdata, elsize, i](){
								work(jobdata + elsize * i);
								m_status->decrement();
							}
						);
					}
				},
				&task_runner
			);
		}

		signaling_counter transform(
			span_2d<std::complex<float>> const input_buffer,
			span_2d<std::complex<float>> output_buffer,
			dft_direction direction
		) const
		{
			auto plan = [this](span_2d_extents extents, dft_direction direction){
				std::lock_guard lock{m_plan_cache_mtx};
				return m_plan_cache.get_plan(extents, direction);
			}(input_buffer.extents(), direction);

			signaling_counter ret{0};
			m_status = &ret.get_state();
			plan.execute(input_buffer.data(), output_buffer.data());
			return ret;
		}

	private:
		static thread_local signaling_counter::semaphore* m_status;

		mutable std::mutex m_plan_cache_mtx;
		mutable dft_execution_plan_cache m_plan_cache;
	};
}

#endif