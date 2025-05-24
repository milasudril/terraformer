//@	{"dependencies_extra":[{"ref":"./contour_plot.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_CONTOUR_PLOT_HPP
#define TERRAFORMER_UI_WIDGETS_CONTOUR_PLOT_HPP

#include "./basic_image_view.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

namespace terraformer::ui::widgets
{
	class contour_plot:public basic_image_view<float, contour_plot>
	{
	public:
		using basic_image_view<float, contour_plot>::handle_event;
		using basic_image_view<float, contour_plot>::compute_size;
		using basic_image_view<float, contour_plot>::confirm_size;
		using basic_image_view<float, contour_plot>::show_image;
		using basic_image_view<float, contour_plot>::prepare_for_presentation;

		contour_plot() = default;

		explicit contour_plot(float dz):
			m_dz{dz}
		{ }

		image apply_filter(span_2d<float const> image) const;

	private:
		float m_dz;
	};
}

#endif
