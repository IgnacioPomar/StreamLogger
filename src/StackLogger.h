/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef STACKLOGGER_H
#	define STACKLOGGER_H

#	include <list>
#	include <string>
#	include <fstream>
#	include <chrono>

#	include <functional>
#	include <mutex>

#	include "StreamLogger/StreamLoggerInterfaces.h"
#	include "StreamLogger/StreamLoggerConsts.h"
#	include "EventContainer.h"
#	include "StackLoggerConfig.h"

namespace IgnacioPomar::Util::StreamLogger
{

	class EventSubscriber
	{
		public:
			EventSubscriber (LogEventsSubscriber &subscriber, const LogLevel logLevel);
			LogEventsSubscriber &subscriber;
			const LogLevel logLevel;
	};

	/**
	 * A logger wich stores the events in a stack
	 */
	class StackLogger : public StackLoggerConfig
	{
		private:
			std::list<EventContainer> events;
			std::list<EventSubscriber> subscribers;

			std::ofstream logfile;

			// Prevent illegal usage
			StackLogger (const StackLogger &)            = delete;    // no copies
			StackLogger &operator= (const StackLogger &) = delete;    // no self-assignments
			StackLogger (StackLogger &&)                 = delete;    // no move constructor
			StackLogger &operator= (StackLogger &&)      = delete;    // no move assignments

			void sendToConsole (EventContainer &event, bool useTimed);
			void sendToFile (EventContainer &event, bool useTimed);
			void sendToSubscribers (EventContainer &event, bool useTimed);

			void storeAndProcess (LogLevel logLevel, std::string &event);

		protected:
			void cleanExcedentEvents ();

		public:
			StackLogger();
			~StackLogger();

			void fillEvent (EventContainer &event, std::string &eventTxt);
			void fillElapsedTime (EventContainer &event);
			void processEvent (EventContainer &event);

			// Without lock: the definitive check is done in log()
			bool isEnabled (LogLevel logLevel) const noexcept;

			// void delLogsOltherThan (int maxLogFileDays);

			// The virtual methods are the entry points: the MT safe version locks them
			virtual void log (LogLevel logLevel, std::string &event);
			virtual void sendEvents (LogEventsSubscriber &receiver, LogLevel logLevel);
			virtual void subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel);
			virtual void unsubscribePushEvents (LogEventsSubscriber &receiver);

			virtual EventContainer &emplaceEvent (LogLevel logLevel);
			virtual void startTimedEvent (EventContainer &event, std::string &eventTxt);
			virtual void appendToTimedEvent (EventContainer &event, const std::string &eventTxt);
			virtual void finishTimedEvent (EventContainer &event);

			// Used to change the configuration
			virtual void runLocked (const std::function<void()> &action);
	};

	class StackLoggerMTSafe : public StackLogger
	{
		private:
			std::mutex mtx;

			// Prevent illegal usage: this class is a singleton
			StackLoggerMTSafe (const StackLoggerMTSafe &)            = delete;    // no copies
			StackLoggerMTSafe &operator= (const StackLoggerMTSafe &) = delete;    // no self-assignments
			StackLoggerMTSafe (StackLoggerMTSafe &&)                 = delete;    // no move constructor
			StackLoggerMTSafe &operator= (StackLoggerMTSafe &&)      = delete;    // no move assignments

		public:
			// Wee need the constructor to be public, as this class is a singleton
			StackLoggerMTSafe();

			// Dont need to override destructor: we dont need mutex as this calss is a singelton, and we will use the
			// base class destructor
			//~StackLoggerMTSafe();

			void log (LogLevel logLevel, std::string &event) override;
			void sendEvents (LogEventsSubscriber &receiver, LogLevel logLevel) override;
			void subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel) override;
			void unsubscribePushEvents (LogEventsSubscriber &receiver) override;

			EventContainer &emplaceEvent (LogLevel logLevel) override;
			void startTimedEvent (EventContainer &event, std::string &eventTxt) override;
			void appendToTimedEvent (EventContainer &event, const std::string &eventTxt) override;
			void finishTimedEvent (EventContainer &event) override;

			void runLocked (const std::function<void()> &action) override;
	};

	StackLogger &getLogger ();

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // STACKLOGGER_H
