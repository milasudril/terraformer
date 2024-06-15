#ifndef TERRAFORMER_UI_WIDGETS_GENERIC_TEXTURE_HPP
#define TERRAFORMER_UI_WIDGETS_GENERIC_TEXTURE_HPP

#include "lib/any/unique_any.hpp"
#include "lib/any/shared_any.hpp"

namespace terraformer::ui::widgets
{
	template<class LifetimeManager>
	class generic_texture
	{
	public:
		generic_texture() = default;

		template<class T, class ... Args>
		requires(!std::is_same_v<std::remove_const_t<T>, generic_texture>)
		explicit generic_texture(std::type_identity<T>, Args&&... args):
			m_pointer{std::type_identity<T>{}, std::forward<Args>(args)...},
			m_upload{
				[](any_pointer_to_mutable ptr, span_2d<rgba_pixel const> pixels){
					ptr.get_if<T>()->upload(pixels);
				}
			}
		{}

		template<class T>
		generic_texture& operator=(T&& src)
		{
			*this = generic_texture{std::type_identity<T>{}, std::forward<T>(src)};
			return *this;
		};

		template<class T>
		T* get_if() const noexcept
		{ return m_pointer.template get_if<T>(); }

		auto get() const noexcept
		{ return m_pointer.get(); }

		auto get_const() const noexcept
		{ return m_pointer.get_const(); }

		void reset() noexcept
		{
			m_pointer.reset();
			m_upload = noop;
		}

		size_t use_count() const noexcept
		requires controls_shared_resource<LifetimeManager>
		{ return m_pointer.use_count(); }

		operator bool() const noexcept
		{ return static_cast<bool>(m_pointer); }

		intptr_t object_id() const
		{ return m_pointer.object_id(); }

		void upload(span_2d<rgba_pixel const> pixels)
		{ m_upload(m_pointer.get(), pixels); }

	private:
		any_smart_pointer<LifetimeManager> m_pointer;

		static void noop(any_pointer_to_mutable, span_2d<rgba_pixel const>){};
		void (*m_upload)(any_pointer_to_mutable, span_2d<rgba_pixel const>) = noop;
	};

	using generic_unique_texture = generic_texture<unique_any_holder>;
	using generic_shared_texture = generic_texture<shared_any_holder>;
}

#endif