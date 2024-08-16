#ifndef TERRAFORMER_UI_MAIN_GENERIC_TEXTURE_HPP
#define TERRAFORMER_UI_MAIN_GENERIC_TEXTURE_HPP

#include "lib/any/unique_any.hpp"
#include "lib/any/shared_any.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/common/span_2d.hpp"

namespace terraformer::ui::main
{
	template<class PointerType>
	class generic_texture
	{
	public:
		generic_texture() = default;

		using callback = void (*)(any_pointer_to_mutable, span_2d<rgba_pixel const>);

		explicit generic_texture(any_pointer_to_mutable ptr, callback upload_function)
		requires std::is_same_v<PointerType, any_pointer_to_mutable>:
			m_pointer{ptr},
			m_upload{upload_function}
		{
		}

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
		requires(std::is_same_v<PointerType, any_pointer_to_mutable>)
		explicit generic_texture(T* ptr):
			m_pointer{ptr},
			m_upload{
				[](any_pointer_to_mutable ptr, span_2d<rgba_pixel const> pixels){
					ptr.get_if<T>()->upload(pixels);
				}
			}
		{}

		template<class T>
		generic_texture& operator=(T&& src)
		{
			if constexpr(std::is_same_v<PointerType, any_pointer_to_mutable>)
			{ *this = generic_texture{std::forward<T>(src)}; }
			else
			{ *this = generic_texture{std::type_identity<T>{}, std::forward<T>(src)}; }
			return *this;
		};

		template<class T>
		T* get_if() const noexcept
		{ return m_pointer.template get_if<T>(); }

		auto get_stored_any() const noexcept
		{
			if constexpr (std::is_same_v<PointerType, any_pointer_to_mutable>)
			{ return m_pointer; }
			else
			{ return m_pointer.get(); }
		}

		auto get() const noexcept
		{
			return generic_texture<any_pointer_to_mutable>{
				get_stored_any(),
				m_upload
			};
		}

		auto get_stored_any_const() const noexcept
		{
			if constexpr (std::is_same_v<PointerType, any_pointer_to_mutable>)
			{ return any_pointer_to_const{m_pointer}; }
			else
			{ return m_pointer.get_const(); }
		}

		void reset() noexcept
		{
			m_pointer.reset();
			m_upload = nullptr;
		}

		size_t use_count() const noexcept
		requires controls_shared_resource<typename PointerType::holder>
		{ return m_pointer.use_count(); }

		operator bool() const noexcept
		{ return static_cast<bool>(m_pointer); }

		intptr_t object_id() const
		{  return m_pointer.object_id(); }

		void upload(span_2d<rgba_pixel const> pixels)
		{ m_upload(get_stored_any(), pixels); }

	private:
		PointerType m_pointer;
		callback m_upload = nullptr;
	};

	using generic_unique_texture = generic_texture<any_smart_pointer<unique_any_holder<false>>>;
	using generic_shared_texture = generic_texture<any_smart_pointer<shared_any_holder<false>>>;
	using generic_texture_pointer = generic_texture<any_pointer_to_mutable>;
}

#endif