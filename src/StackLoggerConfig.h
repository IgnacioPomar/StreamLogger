/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef _STACK_LOGGER_CONFIG_H_
#	define _STACK_LOGGER_CONFIG_H_

#	include <list>
#	include <string>
#	include <fstream>
#	include <chrono>

#	include <mutex>

#	include "StreamLoggerInterfaces.h"
#	include "StreamLoggerConsts.h"
#	include "EventContainer.h"

namespace IgnacioPomar::Util::StreamLogger
{
	class StackLoggerConfig
	{
		public:    // methods
			StackLoggerConfig();

			void setStackSize (unsigned int stackSize);

			void setOutFile (const std::string fileName);    // It'll rotate each day if the template has a %d
			void setOutPath (const std::string filePath);
			void setLevelColor (LogLevel logLevel, LogColor logColor);

			void setConsoleLevel (LogLevel logLevel);
			void setFileLevel (LogLevel logLevel);
			void setStackLevel (LogLevel logLevel);

			void resetSubscriberLevel ();
			void addSubscriberLevel (LogLevel logLevel);

		private:
			void setEffectiveLevel ();

		protected:
			virtual void cleanExcedentEvents () = 0;

		public:    // properties
			LogColor levelColors [6];

			LogLevel consoleLevel;
			LogLevel fileLevel;
			LogLevel stackLevel;
			LogLevel subscriberLevel;
			LogLevel effectiveLevel;

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
	};
}    // namespace IgnacioPomar::Util::StreamLogger

#endif    // _STACK_LOGGER_CONFIG_H_
