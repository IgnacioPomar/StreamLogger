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

#	include <string>
#	include "StreamLoggerConsts.h"

namespace IgnacioPomar::Util::StreamLogger
{

	//--------------  Logger configuration ----------------

	class LGGR_API Config
	{
		public:
			// If 0, there will be no stack at all
			static void setStackSize (unsigned int stackSize);

			static void setOutFile (const std::string fileName);    // It'll rotate by day if the template has a %d
			static void setOutPath (const std::string filePath);
			static void setLevelColor (LogLevel logLevel, LogColor logColor);

			static void setConsoleLevel (LogLevel logLevel);
			static void setFileLevel (LogLevel logLevel);
			static void setStackLevel (LogLevel logLevel);
	};

	//-------------- Classes to use externally ----------------

	class BaseStreamLogger;    // Forward declaration for this functions
	class LogMessageBuilder
	{
		public:
			LGGR_API LogMessageBuilder (BaseStreamLogger &logger);
			LGGR_API LogMessageBuilder (const LogMessageBuilder &other) = delete;
			LGGR_API LogMessageBuilder (LogMessageBuilder &&other) noexcept;
			LGGR_API ~LogMessageBuilder();

			LGGR_API LogMessageBuilder &operator<< (const std::string &value);
			LGGR_API LogMessageBuilder &operator<< (int value);
			LGGR_API LogMessageBuilder &operator<< (double value);

		private:
			BaseStreamLogger &logger;

			// YAGNI: consider use a stringStream, but, worth to add additioonal dependencies?
			std::string message;
	};

	class LGGR_API BaseStreamLogger
	{
		public:
			BaseStreamLogger (LogLevel level);

			const LogLevel level;

			LogMessageBuilder operator<< (const std::string &value);
			LogMessageBuilder operator<< (int value);
			LogMessageBuilder operator<< (double value);

			void log (std::string &message);
	};

	extern LGGR_API BaseStreamLogger trace;
	extern LGGR_API BaseStreamLogger debug;
	extern LGGR_API BaseStreamLogger info;
	extern LGGR_API BaseStreamLogger warn;
	extern LGGR_API BaseStreamLogger error;
	extern LGGR_API BaseStreamLogger fatal;

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // _STREAM_LOGGER_H_
