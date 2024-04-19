/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef _STREAM_LOGGER_H_
#	define _STREAM_LOGGER_H_

// If the lggrExportCfg.h is present, we are using it as a DLL os a static library
// If not, we are using the library as a embedded source
#	if __has_include("lggrExportCfg.h")
#		include "lggrExportCfg.h"
#	else
#		define LGGR_API
#	endif

#	include <sstream>
#	include <string>
#	include "StreamLoggerConsts.h"
#	include "StreamLoggerInterfaces.h"

namespace IgnacioPomar::Util::StreamLogger
{

	//--------------  Logger configuration ----------------

	namespace Config
	{
		// Set this before any threads are started and do not change it afterwards.
		LGGR_API void setMultiThreadSafe (bool multiThreadSafe);

		// If 0, there will be no stack at all
		LGGR_API void setStackSize (unsigned int stackSize);

		LGGR_API void setOutFile (const std::string fileName);    // It'll rotate each day if the template has a %d
		LGGR_API void setOutPath (const std::string filePath);
		LGGR_API void setLevelColor (LogLevel logLevel, LogColor logColor);

		LGGR_API void setConsoleLevel (LogLevel logLevel);
		LGGR_API void setFileLevel (LogLevel logLevel);
		LGGR_API void setStackLevel (LogLevel logLevel);
	};    // namespace Config

	//-------------- Classes to use externally ----------------

	// forward declarations
	class LogMessageBuilder;
	class EventContainer;

	/**
	 * Interfaz to fill the logger message with stream
	 */
	class BaseStreamLogger
	{
		public:
			virtual void log (std::string &message) = 0;
			template <typename T> friend LogMessageBuilder operator<< (BaseStreamLogger &logger, const T &value);
	};

	/**
	 * A Event wich has been already stored in the stack. Allos to count its time
	 * Uppon destruction the event finishes
	 */
	class LGGR_API TimedEvent : public BaseStreamLogger
	{
		private:
			EventContainer &event;
			bool started = false;

		public:
			~TimedEvent();
			TimedEvent (EventContainer &event);
			void log (std::string &message);
	};

	/**
	 * Main interface to log messages: it'll be multiple instances of this class
	 */
	class LGGR_API StaticLogger : public BaseStreamLogger
	{
		public:
			StaticLogger (LogLevel level);

			const LogLevel level;

			void log (std::string &message);

			TimedEvent startTimedEvent ();
	};

	//-------------- Instances of the loggers ----------------
	extern LGGR_API StaticLogger trace;
	extern LGGR_API StaticLogger debug;
	extern LGGR_API StaticLogger info;
	extern LGGR_API StaticLogger warn;
	extern LGGR_API StaticLogger error;
	extern LGGR_API StaticLogger fatal;

	//-------------- template functions ----------------

	template <typename T> LogMessageBuilder operator<< (BaseStreamLogger &logger, const T &value)
	{
		LogMessageBuilder tmpBuilder (logger);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}

	class LogMessageBuilder
	{
		public:
			LogMessageBuilder (BaseStreamLogger &logger)
			    : logger (logger) {};
			LogMessageBuilder (const LogMessageBuilder &other) = delete;
			LogMessageBuilder (LogMessageBuilder &&other) noexcept
			    : logger (other.logger)
			    , message (std::move (other.message)) {};
			~LogMessageBuilder()
			{
				if (this->message.rdbuf()->in_avail() > 0)
				{
					std::string msg = message.str();
					logger.log (msg);
				}
			};

			// this funtion is a template: its outside the dll

			template <typename T> LogMessageBuilder &operator<< (const T &msg)
			{
				this->message << msg;
				return *this;
			}

		private:
			BaseStreamLogger &logger;

			std::stringstream message;
	};

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // _STREAM_LOGGER_H_
