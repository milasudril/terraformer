//@	{"dependencies_extra":[{"ref":"./widget_reference.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_MAIN_WIDGET_REFERENCE_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_REFERENCE_HPP

#include "./widget_collection_ref.hpp"

#include "lib/array_classes/single_array.hpp"

namespace terraformer::ui::main
{
	class widget_reference
	{
	public:
		explicit widget_reference(widget_collection_view collection, widget_collection_view::index_type index):
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

	void flatten(widget_reference const& widget, single_array<widget_reference>& ret);

	single_array<widget_reference> flatten(widget_collection_view const& widgets);

	inline auto find(find_recursive_result const& res, span<widget_reference const> widgets)
	{
		auto const i = std::ranges::find_if(
			widgets,
			[what = res.pointer()](auto const& item){
				auto const widget_pointers = item.collection().widget_pointers();
				auto const index = item.index();

				return widget_pointers[index] == what;
			}
		);

		if(i == std::end(widgets))
		{ return span<widget_reference const>::npos; }

		return span<widget_reference const>::first_element_index() + (i - std::begin(widgets));
	}
}

#endif