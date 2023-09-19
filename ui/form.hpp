#ifndef TERRAFORMER_FORM_HPP
#define TERRAFORMER_FORM_HPP

#include "lib/formbuilder/formfield.hpp"
#include "lib/common/utils.hpp"

#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

namespace terraformer
{
	class widget_row:public QWidget
	{
	public:
		explicit widget_row(QWidget* parent):
			QWidget{parent},
			m_root{std::make_unique<QHBoxLayout>(this)}
			{ m_root->setContentsMargins(0, 0, 0, 0); }
			
			void add_widget(QWidget& widget)
			{ m_root->addWidget(&widget); }
			
	private:
		std::unique_ptr<QHBoxLayout> m_root;
	};
	
	class widget_column:public QWidget
	{
	public:	
		explicit widget_column(QWidget* parent):
			QWidget{parent},
			m_root{std::make_unique<QVBoxLayout>(this)}
			{ m_root->setContentsMargins(4, 0, 0, 0); }
			
			void add_widget(QWidget& widget)
			{ m_root->addWidget(&widget); }
			
	private:
		std::unique_ptr<QVBoxLayout> m_root;
	};
	
	class form:public QWidget
	{
	public:
		explicit form(QWidget* parent):
			QWidget{parent},
			m_root{std::make_unique<QFormLayout>(this)}
		{ m_root->setContentsMargins(4, 0, 0, 0); }

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
			auto entry = create_widget(std::move(field.widget), *this);
			entry->setObjectName(field.name);
			entry->setToolTip(field.description);
			m_root->addRow(field.display_name, entry.get());
			m_widgets.push_back(std::move(entry));
		}

		template<class BindingType>
		void insert(field<subform<BindingType>>&& field)
		{
			auto outer = std::make_unique<widget_column>(this);
			auto label = std::make_unique<QLabel>(field.display_name, outer.get());
			auto entry = create_widget(std::move(field.widget), *outer);
			entry->setObjectName(field.name);
			entry->setToolTip(field.description);
			outer->add_widget(*label);
			outer->add_widget(*entry);
			
			m_root->addRow(outer.get());
			m_widgets.push_back(std::move(label));
			m_widgets.push_back(std::move(entry));
			m_widgets.push_back(std::move(outer));
		}
		
		template<class Converter, class BindingType>
		std::unique_ptr<QLineEdit> create_widget(textbox<Converter, BindingType> const& textbox, QWidget& parent)
		{
			auto ret = std::make_unique<QLineEdit>(&parent);
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
		
		template<class Generator, class BindingType>
		std::unique_ptr<QPushButton> create_widget(input_button<Generator, BindingType>&& input_button, QWidget& parent)
		{
			auto ret = std::make_unique<QPushButton>(input_button.label, &parent);
			ret->setToolTip(input_button.description);
			QObject::connect(ret.get(),
				&QPushButton::clicked,
				[this, &src = *ret, input_button = std::move(input_button), has_been_called = false]() mutable{
					if(has_been_called)
					{ return; }
					has_been_called = true;
					try_and_catch([this, &src](auto const& error){
						log_error(error.what());
						src.setFocus();
					}, [this](auto& input_button){
						input_button.binding.get() = input_button.value_generator();
						refresh();
					}, input_button);
					has_been_called = false;
					refresh();
				}
			);
			return ret;
		}

		template<class Converter, class BindingType>
		std::unique_ptr<QLabel> create_widget(text_display<Converter, BindingType>&& text_display, QWidget& parent)
		{
			auto ret = std::make_unique<QLabel>(&parent);
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
		std::unique_ptr<form> create_widget(subform<BindingType>&& subform, QWidget& parent)
		{
			auto ret = std::make_unique<form>(&parent);
			bind(*ret, subform.binding.get());
			m_display_callbacks.push_back([&ret = *ret](){
				ret.refresh();
			});
			return ret;
		}
		
		template<class... WidgetTypes>
		std::unique_ptr<widget_row> create_widget(std::tuple<WidgetTypes...>&& widgets, QWidget& parent)
		{
			auto ret = std::make_unique<widget_row>(&parent);
			auto created_widgets = std::apply([this, parent = ret.get()]<class... Args>(Args&&... args){
				return std::tuple{create_widget(std::forward<Args>(args), *parent)...};
			}, std::move(widgets));

			std::apply([this, parent = ret.get()]<class... Args>(Args&&... args) {
				(parent->add_widget(*args), ...);
				(m_widgets.push_back(std::forward<Args>(args)), ...);
			}, std::move(created_widgets));

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
