//@	{"dependencies_extra":[{"ref":"./widget_reference.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_MAIN_WIDGET_REFERENCE_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_REFERENCE_HPP

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
			keyboard_button_callback const
		>;
		using widget_array = compatible_multi_array_t<widget_span>;

		using index_type = widget_array::index_type;
		static constexpr index_type npos{static_cast<size_t>(-1)};

		static constexpr auto first_element_index()
		{ return widget_array::first_element_index(); }

		auto last_element_index() const
		{ return m_array.last_element_index(); }

		template<class ... Args>
		flat_widget_collection_view& append(Args&&... args)
		{
			m_array.push_back(std::forward<Args>(args)...);
			return *this;
		}

		auto attributes() const
		{ return m_array.attributes(); }

	private:
		widget_array m_array;
	};

	void flatten(widget_tree_address const& widget, flat_widget_collection_view& ret);

	flat_widget_collection_view flatten(widget_collection_view const& widgets);

	inline auto find(find_recursive_result const& res, flat_widget_collection_view::widget_span span)
	{
		auto const widget_pointers = span.get_by_type<void*>();
		auto const i = std::ranges::find(widget_pointers, res.pointer());

		if(i == std::end(widget_pointers))
		{ return flat_widget_collection_view::npos; }

		return flat_widget_collection_view::first_element_index() + (i - std::begin(widget_pointers));
	}

	inline auto find(find_recursive_result const& res, flat_widget_collection_view const& widgets)
	{ return find(res, widgets.attributes()); }
}

#endif