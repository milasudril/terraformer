#ifndef TERRAFORMER_GLOBAL_INSTANCE_COUNTER_HPP
#define TERRAFORMER_GLOBAL_INSTANCE_COUNTER_HPP

#include <atomic>

namespace terraformer
{
	class global_instance_counter
	{
	public:
		global_instance_counter(global_instance_counter const&):
			m_object_id{s_current_value}
		{ ++s_current_value; }

		global_instance_counter(global_instance_counter&&):
			m_object_id{s_current_value}
		{ ++s_current_value; }

		global_instance_counter():
			m_object_id{s_current_value}
		{ ++s_current_value; }

		global_instance_counter& operator=(global_instance_counter const&) = default;

		global_instance_counter& operator=(global_instance_counter&&) = default;

		uint64_t get_global_id() const
		{ return m_object_id; }

	private:
		inline static std::atomic<uint64_t> s_current_value{0};
		uint64_t m_object_id;
	};
}

#endif