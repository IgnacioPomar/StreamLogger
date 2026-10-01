/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef _STACK_LOGGER_CONFIG_H_
#	define _STACK_LOGGER_CONFIG_H_

#	include <atomic>
#	include <list>
#	include <string>
#	include <fstream>
#	include <chrono>

#	include <mutex>

#	include "StreamLogger/StreamLoggerInterfaces.h"
#	include "StreamLogger/StreamLoggerConsts.h"
#	include "EventContainer.h"

namespace IgnacioPomar::Util::StreamLogger
{
	/**
	 * When to flush an output: after N events of a level, or when the last flush is older than the interval
	 * (checked when writing an event)
	 */
	class FlushPolicy
	{
		public:
			FlushPolicy (const unsigned int (&every) [6], std::chrono::milliseconds interval);

			void setEvery (LogLevel logLevel, unsigned int events);

			// Counts the written event: true if the output must be flushed
			bool countEvent (LogLevel logLevel, TimePoint now);
			void flushed (TimePoint now);

			unsigned int every [6];      // 0: never by count
			unsigned int pending [6];    // Events written since the last flush
			std::chrono::milliseconds interval;    // 0: disabled
			TimePoint lastFlush;
	};

	class StackLoggerConfig
	{
		public:    // methods
			StackLoggerConfig();

			void setStackSize (unsigned int stackSize);

			void setOutFile (const std::string fileName);    // It'll rotate each day if the template has a %d
			void setOutPath (const std::string filePath);
			void setLevelColor (LogLevel logLevel, LogColor logColor);
			void setColorMode (ColorMode colorMode);

			void setConsoleLevel (LogLevel logLevel);
			void setFileLevel (LogLevel logLevel);
			void setStackLevel (LogLevel logLevel);

			void setFlushEvery (LogLevel logLevel, unsigned int events);
			void setFlushInterval (std::chrono::milliseconds interval);
			void setConsoleFlushEvery (LogLevel logLevel, unsigned int events);
			void setConsoleFlushInterval (std::chrono::milliseconds interval);

			void resetSubscriberLevel ();
			void addSubscriberLevel (LogLevel logLevel);

		private:
			void setEffectiveLevel ();

		protected:
			virtual void cleanExcedentEvents () = 0;

		public:    // properties
			LogColor levelColors [6];
			bool useColors;    // Resolved from the ColorMode

			LogLevel consoleLevel;
			LogLevel fileLevel;
			LogLevel stackLevel;
			LogLevel subscriberLevel;
			// Read without lock, to discard the messages before formatting them
			std::atomic<LogLevel> effectiveLevel;

			// In the current implementation, the Timed Events are, while running, in the stack
			// That means that it can have more than maxStoredEvents events
			// And that the stack may contain lower level events than the stackLevel
			// There is no unlimited stack: 0 means no stack
			// YAGNI: Consider extract the timed events to a aditional list
			unsigned int maxStoredEvents;

			std::chrono::year_month_day lastLogDate;
			std::string logPath;
			std::string logFilename;
			std::string logFilePattern;
			bool hasRotation;

			// See DEFAULTS::FLUSH_EVERY and DEFAULTS::CONSOLE_FLUSH_EVERY
			FlushPolicy fileFlush;
			FlushPolicy consoleFlush;
	};
}    // namespace IgnacioPomar::Util::StreamLogger

#endif    // _STACK_LOGGER_CONFIG_H_
