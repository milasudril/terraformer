#ifndef TERRAFORMER_GLOBAL_INSTANCE_COUNTER_HPP
#define TERRAFORMER_GLOBAL_INSTANCE_COUNTER_HPP

#include <atomic>

namespace terraformer::ui::main
{
	class global_instance_counter
	{
	public:
		object_counter(object_counter const&):
			m_object_id{s_current_value}
		{ ++s_current_value; }

		object_counter(object_counter&&):
			m_object_id{s_current_value}
		{ ++s_current_value; }

		object_counter():
			m_object_id{s_current_value}
		{ ++s_object_counter; }

		object_counter& operator=(object_counter const&) = default;

		object_counter& operator=(object_counter&&) = default;

		uint64_t get_global_id() const
		{ return m_object_id; }

	private:
		static std::atomic<uint64_t> s_current_value;
		uint64_t m_object_id;
	};
}

#endif