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
	class form:public QWidget
	{
	public:
		form(QWidget* parent):
			QWidget{parent},
			m_root{std::make_unique<QFormLayout>(this)}
		{}

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
			m_widgets.push_back(create_widget(std::move(field.widget)));
			m_widgets.back()->setObjectName(field.name);
			m_widgets.back()->setToolTip(field.description);
			m_root->addRow(field.display_name, m_widgets.back().get());
		}

		template<class BindingType>
		void insert(subform<BindingType>&& field)
		{
			auto form = create_widget(std::move(field.widget));
			form->setToolTip(field.description);
			form->setObjectName(field.name);
			m_root->addRow(field.display_name, form->m_root);
			m_widgets.push_back(std::move(form));
		}

		template<class Converter, class BindingType>
		std::unique_ptr<QLineEdit> create_widget(textbox<Converter, BindingType> const& textbox)
		{
			auto ret = std::make_unique<QLineEdit>(this);
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
						textbox.binding.get() = textbox.value_converter.convert(str);
						refresh();
					}, src, textbox);
					has_been_called = false;
					refresh();
				}
			);
			m_display_callbacks.push_back([&dest = *ret, textbox](){
				dest.setText(textbox.value_converter.convert(textbox.binding.get()).c_str());
			});

			return ret;
		}

		template<class Converter, class BindingType>
		std::unique_ptr<QLabel> create_widget(text_display<Converter, BindingType>&& text_display)
		{
			auto ret = std::make_unique<QLabel>(this);
			m_display_callbacks.push_back([&dest = *ret, text_display = std::move(text_display)](){
				try_and_catch([](auto const& error){
					log_error(error.what());
				},[](auto& dest, auto const& text_display) {
					dest.setText(text_display.source(text_display.binding.get()).c_str());
				}, dest, text_display);
			});
			return ret;
		}

		template<class BindingType>
		std::unique_ptr<form> create_widget(subform<BindingType>&& subform)
		{
			auto ret = std::make_unique<form>(this);
			bind(*ret, subform.binding.get());
			m_display_callbacks.push_back([&ret = *ret](){
				ret.refresh();
			});
			return ret;
		}

		void refresh() const
		{ std::ranges::for_each(m_display_callbacks, [](auto const& item){item();}); }


	private:
		std::vector<std::unique_ptr<QWidget>> m_widgets;
		std::vector<std::function<void()>> m_display_callbacks;
		std::function<void(char const*)> m_error_handler;
		std::unique_ptr<QFormLayout> m_root;
	};
}

#endif