//@	{"dependencies_extra":[{"ref":"./grayscale_image_statistics_view.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_GRAYSCALE_IMAGE_STATISTICS_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_GRAYSCALE_IMAGE_STATISTICS_VIEW_HPP

#include "./form.hpp"
#include "./label.hpp"
#include "lib/common/span_2d.hpp"
#include <algorithm>

namespace terraformer::ui::widgets
{
	class grayscale_image_statistics_view:private form
	{
	public:
		using form::handle_event;
		using form::prepare_for_presentation;
		using form::theme_updated;
		using form::get_children;
		using form::get_layout;
		using form::compute_size;
		using form::confirm_size;
		using form::layout;

		struct statistic_field_form_field
		{
			std::u8string_view label;
			using input_widget_type = widgets::label;
		};

		explicit grayscale_image_statistics_view(
			iterator_invalidation_handler_ref iihr,
			main::widget_orientation orientation = main::widget_orientation::vertical
		):form{iihr, orientation}
		{
			m_width = &create_widget(statistic_field_form_field{
				.label = u8"Width/px"
			});
			m_height = &create_widget(statistic_field_form_field{
				.label = u8"Height/px"
			});
			m_min_elev = &create_widget(statistic_field_form_field{
				.label = u8"Min elevation/m"
			});
			m_max_elev = &create_widget(statistic_field_form_field{
				.label = u8"Min elevation/m"
			});
			m_max_slope = &create_widget(statistic_field_form_field{
				.label = u8"Max |grad z|"
			});
			layout.params().no_outer_margin = true;
		}

		void show_image(span_2d<float const> pixels, box_size phys_dim);

	private:
		label* m_width;
		label* m_height;
		label* m_min_elev;
		label* m_max_elev;
		label* m_max_slope;
	};
}

#endif