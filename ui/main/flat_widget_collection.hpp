//@	{"dependencies_extra":[{"ref":"./flat_widget_collection.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_MAIN_FLAT_WIDGET_COLLECTION_HPP
#define TERRAFORMER_UI_MAIN_FLAT_WIDGET_COLLECTION_HPP

#include "./widget_collection_ref.hpp"

#include "lib/array_classes/single_array.hpp"

namespace terraformer::ui::main
{
	class widget_tree_address
	{
	public:
		explicit widget_tree_address(widget_collection_view collection, widget_collection_view::index_type index):
			m_collection{collection},
			m_index{index}
		{}

		auto const& collection() const
		{ return m_collection; }

		auto index() const
		{ return m_index; }

	private:
		widget_collection_view m_collection;
		widget_collection_view::index_type m_index;
	};

	class flat_widget_collection_view
	{
	public:
		using widget_span = multi_span<
			void* const,
			widget_tree_address const,
			std::reference_wrapper<widget_state const> const,
			keyboard_button_callback const,
			typing_callback const,
			keyboard_focus_enter_callback const,
			keyboard_focus_leave_callback const
		>;

		using index_type = widget_span::index_type;

		using reference = widget_span::reference;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		flat_widget_collection_view() = default;

		explicit flat_widget_collection_view(widget_span const& span): m_span{span}{}

		static constexpr auto first_element_index()
		{ return widget_span::first_element_index(); }

		auto last_element_index() const
		{ return m_span.last_element_index(); }

		auto size() const
		{ return std::size(m_span); }

		auto widget_pointers() const
		{ return m_span.template get_by_type<void*>(); }

		auto widget_states() const
		{ return m_span.template get_by_type<std::reference_wrapper<widget_state const>>(); }

		template<class EventType, class ... Args>
		auto event_callbacks() const
		{
			if constexpr(m_span.template has_type<event_callback_t<EventType, Args...>>())
			{ return m_span.template get_by_type<event_callback_t<EventType, Args...>>(); }
			else
			{ return m_span.template get_by_type<event_callback_t<EventType const&, Args...>>(); }
		}

		auto get_span() const
		{ return m_span; }

		auto extract(index_type k) const
		{ return m_span[k]; }

		auto addresses() const
		{ return m_span.get_by_type<widget_tree_address>(); }

	private:
		widget_span m_span;
	};

	template<class EventType, class... Args>
	bool try_dispatch(
		EventType&& e,
		flat_widget_collection_view const& view,
		flat_widget_collection_view::index_type index,
		Args&&... args
	)
	{
		if(index == flat_widget_collection_view::npos)
		{ return false; }

		auto const widgets = view.widget_pointers();
		auto const callbacks = view.template event_callbacks<std::remove_cvref_t<EventType>, std::remove_cvref_t<Args>...>();
		callbacks[index](widgets[index], std::forward<EventType>(e), std::forward<Args>(args)...);

		return true;
	}

	class flat_widget_collection
	{
	public:
		using widget_array = compatible_multi_array_t<flat_widget_collection_view::widget_span>;

		using index_type = widget_array::index_type;
		using size_type = widget_array::size_type;
		static constexpr index_type npos{static_cast<size_t>(-1)};

		template<class ... Args>
		flat_widget_collection& append(Args&&... args)
		{
			m_array.push_back(std::forward<Args>(args)...);
			return *this;
		}

		auto attributes() const
		{ return flat_widget_collection_view{m_array.attributes()}; }

		static constexpr auto first_element_index()
		{ return widget_array::first_element_index(); }

		auto last_element_index() const
		{ return m_array.last_element_index(); }

		auto size() const
		{ return m_array.size(); }

	private:
		widget_array m_array;
	};

	void flatten(widget_tree_address const& widget, flat_widget_collection& ret);

	flat_widget_collection flatten(widget_collection_view const& widgets);

	inline auto find(find_recursive_result const& res, flat_widget_collection_view const& span)
	{
		auto const widget_pointers = span.widget_pointers();
		auto const i = std::ranges::find(widget_pointers, res.pointer());

		if(i == std::end(widget_pointers))
		{ return flat_widget_collection_view::npos; }

		return span.first_element_index() + (i - std::begin(widget_pointers));
	}

	inline auto find(find_recursive_result const& res, flat_widget_collection const& widgets)
	{ return find(res, widgets.attributes()); }
}

#endif