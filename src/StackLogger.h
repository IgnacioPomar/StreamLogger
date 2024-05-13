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

#	include <mutex>

#	include "StreamLoggerInterfaces.h"
#	include "StreamLoggerConsts.h"
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

		protected:
			void cleanExcedentEvents ();

		public:
			StackLogger();
			~StackLogger();

			void fillEvent (EventContainer &event, std::string &eventTxt);
			void fillElapsedTime (EventContainer &event);
			void processEvent (EventContainer &event);

			// void delLogsOltherThan (int maxLogFileDays);

			virtual void log (LogLevel logLevel, std::string &event);
			virtual void sendEvents (LogEventsSubscriber &receiver, LogLevel logLevel);
			virtual void subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel);

			virtual EventContainer &emplaceEvent (LogLevel logLevel);
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

			EventContainer &emplaceEvent (LogLevel logLevel) override;
	};

	StackLogger &getLogger ();

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // STACKLOGGER_H
