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

namespace IgnacioPomar::Util::StreamLogger
{
	typedef std::chrono::system_clock::time_point TimePoint;

	/**
	 * Contain the info for a single event
	 */
	class EventContainer
	{
		public:
			// Storing the date as a string to avoid reformating each time we send it
			// And also as timePoint to detect rotation in filename

			TimePoint timePoint;
			std::string date;
			std::string event;
			LogLevel logLevel;
	};

	class EventSubscriber
	{
		public:
			EventSubscriber (const LogEventsSubscriber &subscriber, const LogLevel logLevel);
			const LogEventsSubscriber &subscriber;
			const LogLevel logLevel;
	};

	/**
	 * A logger wich stores the events in a stack
	 */
	class StackLogger
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

			void sendToConsole (EventContainer &event);
			void sendToFile (EventContainer &event);
			void sendToSubscribers (EventContainer &event);
			void fillEvent (EventContainer &event, LogLevel logLevel, std::string &eventTxt);

		protected:
			void setDefaultValues ();
			void cleanExcedentEvents ();

			// YAGNI: Consider extract this vars to a StreamLoggerConfig class, and make StackLogger class to inherit
			// from it
			friend class Config;

			LogColor levelColors [6];
			unsigned int maxStoredEvents;
			LogLevel consoleLevel;
			LogLevel fileLevel;
			LogLevel stackLevel;

			std::chrono::year_month_day lastLogDate;
			std::string logPath;
			std::string logFilename;
			std::string logFilePattern;
			bool hasRotation;

		public:
			StackLogger();
			~StackLogger();
			// void delLogsOltherThan (int maxLogFileDays);

			virtual void log (LogLevel logLevel, std::string &event);
			virtual void sendEvents (LogEventsSubscriber &receiver, LogLevel logLevel);
			virtual void subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel);
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
	};

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // STACKLOGGER_H
