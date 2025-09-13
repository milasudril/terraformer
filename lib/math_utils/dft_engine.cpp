//@	{
//@	"target":{"name":"./dft_engine.o", "type":"object"},
//@	"dependencies":[
//@			{"ref":"fftw3f_threads", "origin":"system", "rel":"external"},
//@			{"ref":"fftw3f", "origin":"pkg-config"}
//@		]
//@	}

#include "./dft_engine.hpp"
#include "lib/common/chunk_by_chunk_count_view.hpp"
#include "lib/array_classes/span.hpp"

#include <algorithm>

namespace
{
	struct plan_deleter
	{
		void operator()(fftwf_plan plan)
		{ if(plan != nullptr) { fftwf_destroy_plan(plan); } }

	};

	terraformer::dft_engine::thread_pool_type* workers;
}


terraformer::dft_execution_plan::dft_execution_plan(size_t size, dft_direction dir)
{
	auto input_buff  = std::make_unique<std::complex<float>[]>(size);
	auto output_buff = std::make_unique<std::complex<float>[]>(size);
	std::fill_n(input_buff.get(), size, 0);
	auto input_buff_ptr  = reinterpret_cast<fftwf_complex*>(input_buff.get());
	auto output_buff_ptr = reinterpret_cast<fftwf_complex*>(output_buff.get());
	m_plan = std::unique_ptr<plan_type, plan_deleter>{fftwf_plan_dft_1d(static_cast<int>(size),
	                                                                  input_buff_ptr,
	                                                                  output_buff_ptr,
	                                                                  static_cast<int>(dir),
	                                                                  FFTW_MEASURE)};
}

terraformer::dft_execution_plan::dft_execution_plan(span_2d_extents size, dft_direction dir)
{
	auto const n = static_cast<size_t>(size.width)*static_cast<size_t>(size.height);
	auto input_buff  = std::make_unique<std::complex<float>[]>(n);
	auto output_buff = std::make_unique<std::complex<float>[]>(n);
	if(workers != nullptr)
	{
		auto const n_workers = workers->max_concurrency();
		for(auto chunk:chunk_by_chunk_count_view{span{input_buff.get(), input_buff.get() + n}, n_workers})
		{ workers->submit([chunk](){ std::ranges::fill(chunk, 0); }); }
	}
	else
	{ std::fill_n(input_buff.get(), n, 0); }
	auto input_buff_ptr  = reinterpret_cast<fftwf_complex*>(input_buff.get());
	auto output_buff_ptr = reinterpret_cast<fftwf_complex*>(output_buff.get());
	m_plan = std::unique_ptr<plan_type, plan_deleter>{
		fftwf_plan_dft_2d(
			size.height,
			size.width,
			input_buff_ptr,
			output_buff_ptr,
			static_cast<int>(dir),
			FFTW_MEASURE
		)
	};
}

terraformer::dft_execution_plan
terraformer::dft_execution_plan_cache::get_plan(sizes buffer_size, dft_direction dir)
{
	auto const i = std::ranges::find(m_transform_sizes, std::pair{buffer_size, dir});
	if(i != std::end(m_transform_sizes)) [[likely]]
	{
		auto const index = i - std::begin(m_transform_sizes);
		auto& plan_info = m_plans[index];
		if(!plan_info.plan)
		{
			plan_info.plan = std::visit([dir](auto buffer_size){
				return dft_execution_plan{buffer_size, dir};
			}, buffer_size);
		}

		plan_info.last_used = m_counter;
		++m_counter;
		return plan_info.plan;
	}

	auto const reject = std::ranges::min_element(m_plans, [](auto const& a, auto const& b) {
		return a.last_used < b.last_used;
	});

	auto const reject_index = reject - std::begin(m_plans);
	reject->plan = std::visit([dir](auto buffer_size){
		return dft_execution_plan{buffer_size, dir};
	}, buffer_size);
	reject->last_used = m_counter;
	++m_counter;
	m_transform_sizes[reject_index] = std::pair{buffer_size, dir};
	return reject->plan;
}

void terraformer::dft_engine::enable_multithreading(thread_pool_type& workers)
{
	fftwf_init_threads();
	fftwf_plan_with_nthreads(static_cast<int>(workers.max_concurrency()));
	fftwf_threads_set_callback(
		[](void *(*work)(char*), char* jobdata, size_t elsize, int njobs, void* workers) {
			signaling_counter counter{static_cast<size_t>(njobs)};
			auto& obj = *static_cast<thread_pool_type*>(workers);
			for (int i = 0; i < njobs; ++i)
			{
				obj.submit(
					[work, jobdata, elsize, i, &counter = counter.get_state()]{
						work(jobdata + elsize * i);
						counter.decrement();
					}
				);
			}
			counter.wait();
		},
		&workers
	);
	::workers = &workers;
}

namespace
{
	constinit thread_local terraformer::dft_execution_plan_cache dft_execution_plans;
}

terraformer::dft_execution_plan terraformer::get_plan(dft_execution_plan_cache::sizes buffer_size, dft_direction dir)
{
	return dft_execution_plans.get_plan(buffer_size, dir);
}