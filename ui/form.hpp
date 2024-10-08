//@	{
//@		"dependencies_extra":[{"ref": "./form.o", "rel":"implementation"}],
//@		"dependencies":[
//@			{"ref":"Qt5Widgets", "origin":"pkg-config"},
//@			{"ref":"Qt5Charts", "origin":"pkg-config"}
//@		]
//@	}

#ifndef TERRAFORMER_FORM_HPP
#define TERRAFORMER_FORM_HPP

#include "lib/formbuilder/formfield.hpp"
#include "lib/common/utils.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/filters/earth_colormap.hpp"
#include "lib/filters/ground_depth_colormap.hpp"

#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDial>
#include <QPushButton>
#include <QPainter>
#include <QApplication>
#include <QTabWidget>
#include <QMouseEvent>
#include <QToolTip>
#include <QChartView>
#include <QComboBox>
#include <QCheckBox>

#include <ranges>

namespace terraformer
{
	static constexpr auto form_indent = 8;
	class widget_row:public QWidget
	{
	public:
		explicit widget_row(QWidget* parent):
			QWidget{parent},
			m_root{std::make_unique<QHBoxLayout>(this)}
			{ m_root->setContentsMargins(0, 0, 0, 0); }

			void add_widget(QWidget& widget)
			{ m_root->addWidget(&widget); }

			void compact_height()
			{
				auto const num_items = m_root->count();
				auto min_height = 32;

				for(int k = 0; k != num_items; ++k)
				{
					auto const item = m_root->itemAt(k);
					auto new_val = item->minimumSize();
					min_height = std::min(min_height, new_val.height());
				}

				for(int k = 0; k != num_items; ++k)
				{
					auto const item = m_root->itemAt(k);
					auto widget = dynamic_cast<QWidgetItem&>(*item).widget();
					widget->setMaximumHeight(min_height);
					if(auto knob = dynamic_cast<QDial*>(widget); knob != nullptr)
					{
						knob->setMaximumWidth(min_height);
					}
				}
			}

		int minimumWidth() const
		{
			auto const num_items = m_root->count();
			auto sum = 0;

			for(int k = 0; k != num_items; ++k)
			{
				auto const item = m_root->itemAt(k);
				sum += item->minimumSize().width();
			}
			return sum;
		}

	private:

		std::unique_ptr<QHBoxLayout> m_root;
	};

	class widget_column:public QWidget
	{
	public:
		explicit widget_column(QWidget* parent, int indent = form_indent):
			QWidget{parent},
			m_root{std::make_unique<QVBoxLayout>(this)}
			{ m_root->setContentsMargins(indent, 0, 0, 0); }

			void add_widget(QWidget& widget)
			{ m_root->addWidget(&widget); }

	private:
		std::unique_ptr<QVBoxLayout> m_root;
	};

	class image_view:public QWidget
	{
	public:
		explicit image_view(QWidget* parent): QWidget{parent}
		{ setMouseTracking(true); }

		void set_pixmap(QPixmap&& pixmap)
		{ m_image_data = std::move(pixmap); }

		template<class Callable>
		void set_mouse_move_callback(Callable&& cb)
		{ m_mouse_move_callback = std::forward<Callable>(cb);  }

	private:
		void paintEvent(QPaintEvent*) override
		{
			QPainter p{this};
			p.drawPixmap(0, 0, m_image_data);
		}

		QPixmap m_image_data;
		std::function<void(QMouseEvent const& event)> m_mouse_move_callback;

		void mouseMoveEvent(QMouseEvent* event) override
		{
			if(m_mouse_move_callback)
			{ m_mouse_move_callback(*event); }
		}
	};

	class colorbar:public QWidget
	{
	public:
		static constexpr auto scale_width = 32;
		static constexpr auto scale_min_height = 8*scale_width;

		explicit colorbar(QWidget* parent): QWidget{parent}
		{
			auto const app_font = QApplication::font(this);
			QFontMetrics fm{app_font};
			m_label_width = fm.horizontalAdvance("99999");
			m_em = fm.horizontalAdvance("m");
			m_label_height = fm.lineSpacing();
			m_digit_height = fm.ascent();

			setMinimumWidth(scale_width + m_label_width + 2*m_em);
			setMinimumHeight(scale_min_height + m_label_height);
			m_range = std::ranges::minmax_result{0.0f, 1.0f};
		}

		void set_colormap(std::span<rgba_pixel const> colormap)
		{
			m_colormap = std::vector<rgba_pixel>{std::begin(colormap), std::end(colormap)};
			generate_image();
		}

		void set_range(std::ranges::minmax_result<float> range)
		{ m_range = range; }

		void redraw()
		{
			generate_image();
			update();
		}

	private:
		void showEvent(QShowEvent*) override
		{ redraw(); }

		void generate_image();

		void paintEvent(QPaintEvent*) override;

		QPixmap m_image_data;
		std::ranges::minmax_result<float> m_range;
		int m_label_width;
		int m_em;
		int m_label_height;
		int m_digit_height;
		std::vector<rgba_pixel> m_colormap;
	};

	class topographic_map_view_map_view:public QWidget
	{
	public:
		explicit topographic_map_view_map_view(QWidget* parent);

		void upload(std::reference_wrapper<grayscale_image const> img,
			float pixel_size,
			std::ranges::minmax_result<float> valid_range);

		void set_colormap(std::span<rgba_pixel const> colormap)
		{
			m_colormap = std::vector<rgba_pixel>{std::begin(colormap), std::end(colormap)};
			m_colorbar->set_colormap(colormap);
		}

		void redraw_colorbar()
		{ m_colorbar->redraw(); }

	private:
		void showEvent(QShowEvent*) override
		{ upload(*m_heightmap, m_pixel_size, m_valid_range); }

		std::unique_ptr<QHBoxLayout> m_root;
		std::vector<rgba_pixel> m_colormap;
		std::unique_ptr<image_view> m_image_view;
		std::unique_ptr<colorbar> m_colorbar;
		grayscale_image const* m_heightmap;
		float m_render_scale;
		float m_pixel_size;
		std::ranges::minmax_result<float> m_valid_range;
	};

	class topographic_map_xsection_diagram:public QWidget
	{
	public:
		enum class axis_direction{north_to_south, west_to_east};

		explicit topographic_map_xsection_diagram(QWidget* parent, axis_direction axis_dir):
			QWidget{parent},
			m_root{std::make_unique<QHBoxLayout>(this)},
			m_axis{std::make_unique<QtCharts::QChartView>(this)},
			m_axis_dir{axis_dir},
			m_colorbar{std::make_unique<colorbar>(this)}
		{
			m_root->setContentsMargins(0, 0, 0, 0);
			m_axis->setSizePolicy(QSizePolicy{
				QSizePolicy::Policy::Expanding,
				QSizePolicy::Policy::Expanding
			});
			m_axis->setMinimumWidth(512);

			m_colorbar->setSizePolicy(QSizePolicy{
				QSizePolicy::Policy::Fixed,
				QSizePolicy::Policy::Expanding
			});
			m_colorbar->setToolTip("Shows mapping between color and location");

			m_root->addWidget(m_axis.get());
			m_root->addWidget(m_colorbar.get());
			m_root->addSpacing(0);
			set_colormap(ground_depth_colormap);
		}

		void upload(grayscale_image const& img,
			float pixel_size,
			std::ranges::minmax_result<float> valid_range);

		void set_colormap(std::span<rgba_pixel const> colormap)
		{
			m_colormap = std::vector<rgba_pixel>{std::begin(colormap), std::end(colormap)};
			m_colorbar->set_colormap(colormap);
		}

		void redraw_colorbar()
		{ m_colorbar->redraw(); }

	private:
		std::unique_ptr<QHBoxLayout> m_root;
		std::unique_ptr<QtCharts::QChartView> m_axis;
		axis_direction m_axis_dir;
		std::unique_ptr<colorbar> m_colorbar;
		std::vector<rgba_pixel> m_colormap;
	};

	class topographic_map_view_xsection_view:public QWidget
	{
	public:
		explicit topographic_map_view_xsection_view(QWidget* parent):
			QWidget{parent},
			m_root{std::make_unique<QVBoxLayout>(this)},
			m_ns_view{std::make_unique<topographic_map_xsection_diagram>(this,
				topographic_map_xsection_diagram::axis_direction::north_to_south
			)},
			m_we_view{std::make_unique<topographic_map_xsection_diagram>(this,
				topographic_map_xsection_diagram::axis_direction::west_to_east),
			}
		{
			m_root->setContentsMargins(form_indent, 0, 0, 0);
			m_ns_view->setSizePolicy(QSizePolicy{
				QSizePolicy::Policy::Expanding,
				QSizePolicy::Policy::Expanding
			});
			m_ns_view->setToolTip("Shows cross-sections from west to east");
			m_root->addWidget(m_ns_view.get());

			m_we_view->setSizePolicy(QSizePolicy{
				QSizePolicy::Policy::Expanding,
				QSizePolicy::Policy::Expanding
			});
			m_we_view->setToolTip("Shows cross-sections from north to south");
			m_root->addWidget(m_we_view.get());
		}

		void upload(grayscale_image const& img, float pixel_size, std::ranges::minmax_result<float> valid_range)
		{
			m_ns_view->upload(img, pixel_size, valid_range);
			m_we_view->upload(img, pixel_size, valid_range);
		}

		void redraw_colorbar()
		{
			m_ns_view->redraw_colorbar();
			m_we_view->redraw_colorbar();
		}

	private:
		std::unique_ptr<QVBoxLayout> m_root;
		std::unique_ptr<topographic_map_xsection_diagram> m_ns_view;
		std::unique_ptr<topographic_map_xsection_diagram> m_we_view;
	};

	class topographic_map_renderer:public QWidget
	{
	public:
		explicit topographic_map_renderer(QWidget* parent):
			QWidget{parent},
			m_root{std::make_unique<QHBoxLayout>(this)},
			m_tabs{std::make_unique<QTabWidget>(this)},
			m_map{new topographic_map_view_map_view{nullptr}},
			m_crossection{new topographic_map_view_xsection_view{nullptr}}
		{
			m_root->addWidget(m_tabs.get());
			m_tabs->addTab(m_map, "Topographic map");
			m_tabs->addTab(m_crossection, "Cross-sections");
		}

		void upload(std::reference_wrapper<grayscale_image const> img,
			float pixel_size,
			std::ranges::minmax_result<float> valid_range)
		{
			m_map->upload(img, pixel_size, valid_range);
			m_crossection->upload(img, pixel_size, valid_range);
		}

		void set_colormap(std::span<rgba_pixel const> colormap)
		{ m_map->set_colormap(colormap); }

		void redraw_colorbar()
		{
			m_map->redraw_colorbar();
			m_crossection->redraw_colorbar();
		}

	private:
		std::unique_ptr<QHBoxLayout> m_root;
		std::unique_ptr<QTabWidget> m_tabs;
		// NOTE: According to the documentation for QTabWidget, a QTabWidget takes ownership
		//       of the object. Thus, we do not use unique_ptr here.
		topographic_map_view_map_view* m_map;
		topographic_map_view_xsection_view* m_crossection;
	};

	inline std::string make_widget_path(std::string const& path, QString const& field_name)
 	{ return std::string{path}.append("/").append(field_name.toStdString()); }

	template<class ValueChangedListener>
	class form:public QWidget
	{
	public:
		template<class ValueChangedListenerType>
		explicit form(QWidget* parent, std::string&& path,
			ValueChangedListenerType&& on_value_changed,
			size_t level = 0):
			QWidget{parent},
			m_on_value_changed{std::forward<ValueChangedListener>(on_value_changed)},
			m_root{std::make_unique<QFormLayout>(this)},
			m_level{level},
			m_path{std::move(path)}
		{ m_root->setContentsMargins(level == 0? 4: form_indent, 0, 0, 0); }

		void set_focus()
		{
			auto first_element = m_widgets[0].get();
			if(auto form = dynamic_cast<class form*>(first_element); form != nullptr)
			{ form->set_focus(); }
			else
			{ first_element->setFocus(); }
		}

		template<class FieldDescriptor>
		void insert(FieldDescriptor&& field)
		{
			auto entry = create_widget(std::move(field.widget), *this, field.name);
			entry->setToolTip(field.description);
			if(entry->minimumWidth() < 128)
			{
				m_root->addRow(field.display_name, entry.get());
				m_widgets.push_back(std::move(entry));
			}
			else
			{
				auto outer = std::make_unique<widget_column>(this, 0);
				auto label = std::make_unique<QLabel>(field.display_name, outer.get());
				outer->add_widget(*label);
				outer->add_widget(*entry);

				m_root->addRow(outer.get());
				m_widgets.push_back(std::move(label));
				m_widgets.push_back(std::move(entry));
				m_widgets.push_back(std::move(outer));
			}
		}

		template<class BindingType>
		void insert(field<subform<BindingType>>&& field)
		{
			auto outer = std::make_unique<widget_column>(this, 0);
			auto label = std::make_unique<QPushButton>(
				QString::fromStdString(std::string{"⊞ "}.append(field.display_name)),
				outer.get()
			);
			auto entry = create_widget(
				std::move(field.widget),
				*outer,
				std::move(std::string{m_path}.append("/").append(field.name)));
			label->setToolTip("Click to see details");
			label->setStyleSheet("border: none; text-align:left; margin:0; padding: 0; font-weight: bold");
			QObject::connect(label.get(),
				&QPushButton::clicked,[
					&entry = *entry,
					&label = *label,
					entry_visible = false,
					raw_name = std::string{field.display_name}
				]() mutable {
					entry.setVisible(!entry_visible);
					entry_visible = !entry_visible;
					if(entry_visible)
					{ label.setText(QString::fromStdString(std::string{"⊟ "}.append(raw_name))); }
					else
					{ label.setText(QString::fromStdString(std::string{"⊞ "}.append(raw_name))); }
				}
			);
			entry->setObjectName(field.name);
			entry->setVisible(false);
			entry->setToolTip(field.description);
			outer->add_widget(*label);
			outer->add_widget(*entry);

			m_root->addRow(outer.get());
			m_widgets.push_back(std::move(label));
			m_widgets.push_back(std::move(entry));
			m_widgets.push_back(std::move(outer));
		}

		template<class BindingType, class Converter>
		std::unique_ptr<QLineEdit>
		create_widget(textbox<BindingType, Converter> const& textbox, QWidget& parent, char const* field_name)
		{
			auto ret = std::make_unique<QLineEdit>(&parent);

			if constexpr(!std::is_const_v<typename BindingType::type>)
			{
				QObject::connect(ret.get(),
					&QLineEdit::editingFinished,
					[this, &src = *ret, textbox, has_been_called = false]() mutable{
						if(has_been_called)
						{ return; }
						has_been_called = true;
						try_and_catch([this, &src](auto const& error){
							log_error(error.what());
							src.setFocus();
						}, [this](auto& src, auto const& textbox){
							auto const str = src.text().toStdString();
							if(auto new_val = textbox.value_converter.convert(str);
								new_val != textbox.binding.get())
							{
								textbox.binding.get() = std::move(new_val);
								m_on_value_changed(make_widget_path(m_path, src.objectName()));
							}
						}, src, textbox);
						refresh();
						has_been_called = false;
					}
				);
			}
			else
			{ ret->setReadOnly(true); }

			m_display_callbacks.push_back([&dest = *ret, textbox](){
				dest.setText(textbox.value_converter.convert(textbox.binding.get()).c_str());
			});
			if(textbox.min_width.has_value())
			{
				auto const app_font = QApplication::font(this);
				QFontMetrics fm{app_font};
				auto const char_width = fm.horizontalAdvance("A");
				ret->setMinimumWidth(char_width*(*textbox.min_width));
			}
			ret->setObjectName(field_name);
			return ret;
		}

		template<class BindingType>
		std::unique_ptr<QCheckBox>
		create_widget(bool_input<BindingType> const& checkbox, QWidget& parent, char const* field_name)
		{
			auto ret = std::make_unique<QCheckBox>(&parent);

			if constexpr(!std::is_const_v<typename BindingType::type>)
			{
				QObject::connect(ret.get(),
					&QCheckBox::stateChanged,
					[this, &src = *ret, checkbox, has_been_called = false]() mutable{
						if(has_been_called)
						{ return; }
						has_been_called = true;
						try_and_catch([this, &src](auto const& error){
							log_error(error.what());
							src.setFocus();
						}, [this](auto& src, auto const& checkbox){
							auto const str = src.text().toStdString();
							if(auto new_val = src.isChecked(); new_val != checkbox.binding.get())
							{
								checkbox.binding.get() = new_val;
								m_on_value_changed(make_widget_path(m_path, src.objectName()));
							}
						}, src, checkbox);
						refresh();
						has_been_called = false;
					}
				);
			}
			else
			{
				// NOTE: Workaround since checkbox has no "readonly" property
				ret->setEnabled(false);
			}

			m_display_callbacks.push_back([&dest = *ret, checkbox](){
				dest.setChecked(checkbox.binding.get());
			});

			ret->setObjectName(field_name);
			return ret;
		}


		template<class BindingType, numeric_input_mapping_type Mapping>
		std::unique_ptr<QDial>
		create_widget(knob<BindingType, Mapping> const& knob, QWidget& parent, char const* field_name)
		{
			auto ret = std::make_unique<QDial>(&parent);
			constexpr auto maxval = 16777216;
			ret->setMinimum(0);
			ret->setMaximum(maxval);

			if constexpr(!std::is_const_v<typename BindingType::type>)
			{
				QObject::connect(ret.get(),
					&QDial::valueChanged,
					[this, &src = *ret, knob, has_been_called = false]() mutable{
						if(has_been_called)
						{ return; }
						has_been_called = true;
						try_and_catch([this, &src](auto const& error){
							log_error(error.what());
							src.setFocus();
						}, [this](auto& src, auto const& knob){
							if constexpr(Mapping == numeric_input_mapping_type::lin)
							{
								if(auto new_val =
									static_cast<BindingType::type::value_type>(std::lerp(
										static_cast<double>(knob.min),
										static_cast<double>(knob.max),
										static_cast<double>(src.value())/maxval));
									new_val != knob.binding.get())
								{
									knob.binding.get() = new_val;
									m_on_value_changed(make_widget_path(m_path, src.objectName()));
								}
							}
							else
							{
								auto const output_min = std::log2(static_cast<double>(knob.binding.get().min()));
								auto const output_max = std::log2(static_cast<double>(knob.binding.get().max()));
								auto const logval = std::lerp(output_min, output_max, static_cast<double>(src.value())/maxval);
								auto const new_val = std::exp2(logval);
								if(new_val != knob.binding.get())
								{
									knob.binding.get() = static_cast<BindingType::type::value_type>(new_val);
									m_on_value_changed(make_widget_path(m_path, src.objectName()));
								}
							}
						}, src, knob);
						refresh();
						has_been_called = false;
					}
				);
			}
			else
			{ ret->setDisabled(true); }

			m_display_callbacks.push_back([&dest = *ret, knob](){
				if constexpr(Mapping == numeric_input_mapping_type::lin)
				{
					auto const val = knob.binding.get();
					auto const val_normalized = static_cast<double>(val - knob.min)/static_cast<double>(knob.max - knob.min);
					dest.setValue(static_cast<int>(static_cast<double>(maxval)*val_normalized + 0.5));
				}
				else
				{
					auto const val = knob.binding.get();
					auto const logval = std::log2(static_cast<double>(val));
					auto const output_min = std::log2(static_cast<double>(knob.binding.get().min()));
					auto const output_max = std::log2(static_cast<double>(knob.binding.get().max()));
					auto const val_normalized = static_cast<double>(logval - output_min)/static_cast<double>(output_max - output_min);
					dest.blockSignals(true);
					dest.setValue(static_cast<int>(static_cast<double>(maxval)*val_normalized + 0.5));
					dest.blockSignals(false);
				}
			});


			ret->setObjectName(field_name);
			return ret;
		}

		template<class BindingType, class Generator>
		std::unique_ptr<QPushButton>
		create_widget(input_button<BindingType, Generator>&& input_button, QWidget& parent, char const* field_name)
		{
			auto ret = std::make_unique<QPushButton>(input_button.label, &parent);
			ret->setToolTip(input_button.description);

			if constexpr(!std::is_const_v<typename BindingType::type>)
			{
				QObject::connect(ret.get(),
					&QPushButton::clicked,
					[this, &src = *ret, input_button = std::move(input_button), has_been_called = false]() mutable{
						if(has_been_called)
						{ return; }
						has_been_called = true;
						try_and_catch([this, &src](auto const& error){
							log_error(error.what());
							src.setFocus();
						}, [this, &src](auto& input_button){
							if(auto new_val = input_button.value_generator(); new_val != input_button.binding.get())
							{
								input_button.binding.get() = std::move(new_val);
								m_on_value_changed(make_widget_path(m_path, src.objectName()));
							}
						}, input_button);
						has_been_called = false;
						refresh();
					}
				);
			}
			else
			{ ret->setDisabled(true); }

			ret->setObjectName(field_name);
			return ret;
		}

		template<class BindingType>
		std::unique_ptr<form> create_widget(subform<BindingType>&& subform,
			QWidget& parent,
			std::string&& name)
		{
			auto ret = std::make_unique<form>(&parent, std::move(name), m_on_value_changed, m_level + 1);
			bind(*ret, subform.binding);
			m_display_callbacks.push_back([&ret = *ret](){
				ret.refresh();
			});
			return ret;
		}

		template<class PixelSizeType, class HeightmapType, class RangeType>
		std::unique_ptr<topographic_map_renderer> create_widget(topographic_map_view<PixelSizeType, HeightmapType, RangeType>&& view,
			QWidget& parent,
			char const* field_name)
		{
			auto ret = std::make_unique<topographic_map_renderer>(&parent);
			m_display_callbacks.push_back([&dest = *ret,
				pixels = view.heightmap,
				pixel_size = view.pixel_size,
				valid_range = view.valid_range](){
				dest.upload(pixels,
					pixel_size,
					std::ranges::minmax_result{
						.min = static_cast<float>(valid_range.get().min()),
						.max = static_cast<float>(valid_range.get().max())
					}
				);
				dest.redraw_colorbar();
			});
			ret->setObjectName(field_name);
			return ret;
		}

		template<class... WidgetTypes>
		std::unique_ptr<widget_row> create_widget(std::tuple<WidgetTypes...>&& widgets, QWidget& parent, char const* field_name)
		{
			auto ret = std::make_unique<widget_row>(&parent);
			auto created_widgets = std::apply([this, parent = ret.get(), field_name]<class... Args>(Args&&... args){
				return std::tuple{create_widget(std::forward<Args>(args), *parent, field_name)...};
			}, std::move(widgets));

			std::apply([this, parent = ret.get()]<class... Args>(Args&&... args) {
				(parent->add_widget(*args), ...);
				(m_widgets.push_back(std::forward<Args>(args)), ...);
			}, std::move(created_widgets));

			ret->compact_height();

			return ret;
		}

		template<class BindingType, class Converter, numeric_input_mapping_type Mapping>
		auto create_widget(numeric_input<BindingType, Converter, Mapping>&& widget, QWidget& parent, char const* field_name)
		{
			return create_widget(std::tuple{
				knob<BindingType, Mapping>{
					.binding = widget.binding,
					.min = widget.binding.get().min(),
					.max = widget.binding.get().max()
				},
				textbox{
					.binding = widget.binding,
					.value_converter = widget.value_converter
				}
			}, parent, field_name);
		}

		template<class BindingType, class Converter, class LabelArray>
		std::unique_ptr<QComboBox>
		create_widget(enum_input<BindingType, Converter, LabelArray> const& enum_input, QWidget& parent, char const* field_name)
		{
			auto ret = std::make_unique<QComboBox>(&parent);
			for(auto const& label : enum_input.labels)
			{ ret->addItem(label); }

			if constexpr(!std::is_const_v<typename BindingType::type>)
			{
				QObject::connect(ret.get(),
					qOverload<int>(&QComboBox::currentIndexChanged),
					[this, &src = *ret, enum_input, has_been_called = false](int selected_index) mutable{
						if(has_been_called)
						{ return; }
						has_been_called = true;

						try_and_catch([this, &src](auto const& error){
							log_error(error.what());
							src.setFocus();
						}, [this](auto& src, auto const& enum_input, int selected_index){
							if(auto new_val = enum_input.value_converter.convert(selected_index);
								new_val != enum_input.binding.get())
							{
								enum_input.binding.get() = std::move(new_val);
								m_on_value_changed(make_widget_path(m_path, src.objectName()));
							}
						}, src, enum_input, selected_index);

						refresh();
						has_been_called = false;
					}
				);
			}
			else
			{ ret->setDisabled(true); }

			m_display_callbacks.push_back([&dest = *ret, enum_input](){
				dest.blockSignals(true);
				dest.setCurrentIndex(enum_input.value_converter.convert(enum_input.binding.get()));
				dest.blockSignals(false);
			});

			ret->setObjectName(field_name);
			return ret;
		}

		void refresh() const
		{ std::ranges::for_each(m_display_callbacks, [](auto const& item){item();}); }

	private:
		[[no_unique_address]] ValueChangedListener m_on_value_changed;
		std::vector<std::unique_ptr<QWidget>> m_widgets;
		std::vector<std::function<void()>> m_display_callbacks;
		std::unique_ptr<QFormLayout> m_root;
		size_t m_level;
		std::string m_path;

		void resizeEvent(QResizeEvent*) override
		{ refresh(); }
	};

	template<class ValueChangedListenerType>
	form(QWidget* parent,
		std::string&& path,
		ValueChangedListenerType&& on_value_changed,
		size_t level = 0) -> form<ValueChangedListenerType>;
}
#endif
