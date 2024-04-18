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

	class LGGR_API Config
	{
		public:
			// Set this before any threads are started and do not change it afterwards.
			static void setMultiThreadSafe (bool multiThreadSafe);

			// If 0, there will be no stack at all
			static void setStackSize (unsigned int stackSize);

			static void setOutFile (const std::string fileName);    // It'll rotate each day if the template has a %d
			static void setOutPath (const std::string filePath);
			static void setLevelColor (LogLevel logLevel, LogColor logColor);

			static void setConsoleLevel (LogLevel logLevel);
			static void setFileLevel (LogLevel logLevel);
			static void setStackLevel (LogLevel logLevel);
	};

	//-------------- Classes to use externally ----------------

	class LogMessageBuilder;
	class LGGR_API BaseStreamLogger
	{
		public:
			BaseStreamLogger (LogLevel level);

			const LogLevel level;

			void log (std::string &message);

			template <typename T> friend LogMessageBuilder operator<< (BaseStreamLogger &logger, const T &value);
	};

	//-------------- Instances of the loggers ----------------
	extern LGGR_API BaseStreamLogger trace;
	extern LGGR_API BaseStreamLogger debug;
	extern LGGR_API BaseStreamLogger info;
	extern LGGR_API BaseStreamLogger warn;
	extern LGGR_API BaseStreamLogger error;
	extern LGGR_API BaseStreamLogger fatal;

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
