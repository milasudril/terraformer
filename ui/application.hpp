//@	{
//@		"dependencies":[{"ref":"Qt5Widgets", "origin":"pkg-config"}]
//@	}

#ifndef TERRAFORMER_APPLICATION_HPP
#define TERRAFORMER_APPLICATION_HPP

#include <functional>

#include <QApplication>

namespace terraformer
{
	class application:public QApplication
	{
	public:
		template<class ... Args>
		explicit application(Args&&... args):
			QApplication{std::forward<Args>(args)...},
			m_internal_event_type{static_cast<QEvent::Type>(QEvent::registerEventType())}
		{}

		struct internal_event:public QEvent
		{
			std::function<bool()> callback;

			template<class Callable>
			explicit internal_event(QEvent::Type event_type, Callable&& f) :
				QEvent{event_type},
				callback{std::forward<Callable>(f)}
			{}

			auto fire() const
			{ return callback(); }
		};

		bool event(QEvent* e) override
		{
			if(e->type() == m_internal_event_type)
			{ return static_cast<internal_event*>(e)->fire(); }
			return QApplication::event(e);
		}

		template<class Callable>
		void post_event(Callable&& callback)
		{
			// NOTE: We use naked new here because Qt will take ownership of the object
			postEvent(this, new internal_event{m_internal_event_type, std::forward<Callable>(callback)});
		}

	private:
		QEvent::Type m_internal_event_type;
	};
}

#endif