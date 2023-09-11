#ifndef TERRAFORMER_FORM_HPP
#define TERRAFORMER_FORM_HPP

#include "lib/formbuilder/formfield.hpp"
#include "lib/common/utils.hpp"

#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>

namespace terraformer
{
	class form
	{
	public:
		form(QWidget* parent):
			m_root{parent}
		{}

		void set_focus()
		{ m_widgets[0]->setFocus(); }

		template<class FieldDescriptor>
		void insert(FieldDescriptor&& field)
		{
			m_widgets.push_back(create_widget(std::move(field.widget)));
			m_widgets.back()->setToolTip(field.description);
			m_root.addRow(field.display_name, m_widgets.back().get());
		}

		template<class Converter, class BindingType>
		std::unique_ptr<QWidget> create_widget(textbox<Converter, BindingType> const& textbox)
		{
			auto ret = std::make_unique<QLineEdit>();
			QObject::connect(ret.get(),
				&QLineEdit::editingFinished,
				[this, &src = *ret, textbox](){
					try_and_catch([&src](auto const& error){
						log_error(error.what());
						src.setFocus();
					}, [this](auto& src, auto const& textbox){
						auto const str = src.text().toStdString();
						textbox.binding.get() = textbox.value_converter.convert(str);
					}, src, textbox);
					refresh();
				}
			);
			m_display_callbacks.push_back([&dest = *ret, textbox](){
				dest.setText(textbox.value_converter.convert(textbox.binding.get()).c_str());
			});

			return ret;
		}

		template<class Converter, class BindingType>
		std::unique_ptr<QWidget> create_widget(text_display<Converter, BindingType>&& text_display)
		{
			auto ret = std::make_unique<QLabel>();
			m_display_callbacks.push_back([&dest = *ret, text_display = std::move(text_display)](){
				try_and_catch([](auto const& error){
					log_error(error.what());
				},[](auto& dest, auto const& text_display) {
					dest.setText(text_display.source(text_display.binding.get()).c_str());
				}, dest, text_display);
			});
			return ret;
		}

		void refresh()
		{ std::ranges::for_each(m_display_callbacks, [](auto const& item){item();}); }


	private:
		std::vector<std::unique_ptr<QWidget>> m_widgets;
		std::vector<std::function<void()>> m_display_callbacks;
		QFormLayout m_root;
		std::function<void(char const*)> m_error_handler;
	};
}

#endif