#ifndef TERRAFORMER_SINGLE_ARRAY_HPP
#define TERRAFORMER_SINGLE_ARRAY_HPP

#include "./memory_block.hpp"
#include "./span.hpp"

namespace terraformer
{
	template<class T>
	class single_array
	{
	public:
		single_array() = default;
		
		~single_array()
		{ clear(); }

		constexpr auto first_element_index() const
		{ return array_index<T>{}; }
		
		auto size() const
		{ return m_size; }

		auto capacity() const
		{ return m_capacity; }

		auto data()
		{ return m_storage.interpret_as<T>(); }

		auto data() const
		{ return m_storage.interpret_as<T const>(); }

		auto begin()
		{ return data(); }

		auto begin() const
		{ return data(); }

		auto end()
		{ return begin() + size().get(); }

		auto end() const
		{ return begin() + size().get(); }

		void reserve(array_size<T> new_capacity)
		{
			if(new_capacity > m_capacity)
			{
				memory_block new_storage{make_byte_size(new_capacity)};
				std::uninitialized_move(begin(), end(), new_storage.interpret_as<T>());
				std::destroy(begin(), end());
				m_storage = std::move(new_storage);
				m_capacity = new_capacity;
			}
		}

		void push_back(T&& elem)
		{
			auto new_size = m_size + terraformer::array_size<T>{1};
			if(new_size > m_capacity)
			{ reserve(std::max(terraformer::array_size<T>{8}, static_cast<size_t>(2)*capacity())); }
			std::construct_at(m_storage.interpret_as<T>() + m_size.get(), std::move(elem));
			m_size = new_size;
		}
		
		void clear()
		{ 
			std::destroy(begin(), end());
			m_size = array_size<T>{};
		}
		
		void resize(array_size<T> new_size);
		
		auto& operator[](array_index<T> index)
		{ return deref(data(), index); }
			
		auto& operator[](array_index<T> index) const
		{ return deref(data(), index); }
		
		operator span<T>()
		{ return span{begin(), end()}; }
		
		operator span<T const>()
		{ return span{end(), end()}; }

	private:
		memory_block m_storage;
		array_size<T> m_size;
		array_size<T> m_capacity;
	};
}

#endif
