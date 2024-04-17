/*********************************************************************************************
 *	Name		: StackLogger.h
 *	Description	: Stack Logger Header
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

#	include "StreamLoggerConsts.h"

namespace StreamLogger
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

	/**
	 * A logger wich stores the events in a stack
	 */
	class StackLogger
	{
		private:
			std::list<EventContainer> events;

			std::chrono::year_month_day lastLogDate;
			std::string logPath;
			std::string logFilename;
			std::string logFilePattern;
			bool hasRotation;

			std::ofstream logfile;

			unsigned int maxStoredEvents;
			LogLevel consoleLevel;
			LogLevel fileLevel;
			LogLevel stackLevel;

			// Prevent illegal usage
			StackLogger (const StackLogger &)            = delete;    // no copies
			StackLogger &operator= (const StackLogger &) = delete;    // no self-assignments
			StackLogger (StackLogger &&)                 = delete;    // no move constructor
			StackLogger &operator= (StackLogger &&)      = delete;    // no move assignments

			void sendToConsole (EventContainer &event);
			void sendToFile (EventContainer &event);
			void fillEvent (EventContainer &event, LogLevel logLevel, std::string &eventTxt);

		protected:
			friend class Config;
			void setStackSize (unsigned int size);
			void setConsoleLevel (LogLevel logLevel);
			void setFileLevel (LogLevel logLevel);
			void setStackLevel (LogLevel logLevel);
			void setFilePattern (const std::string &pattern);
			void setLogPath (const std::string &path);

		public:
			StackLogger();
			~StackLogger();
			// void delLogsOltherThan (int maxLogFileDays);

			void log (LogLevel logLevel, std::string &event);
			void sendEvents (LogEventsReceiver &receiver, LogLevel logLevel);
	};

}    // namespace StreamLogger
#endif    // STACKLOGGER_H
