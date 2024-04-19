/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <string>

#include "StreamLoggerConsts.h"
#include "StackLogger.h"
#include "StreamLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{
	//--------------  Configuration functions ----------------
	void Config::setLevelColor (LogLevel logLevel, LogColor logColor)
	{
		getLogger().levelColors [static_cast<int> (logLevel)] = logColor;
	}

	void Config::setConsoleLevel (LogLevel logLevel)
	{
		getLogger().consoleLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}

	void Config::setFileLevel (LogLevel logLevel)
	{
		getLogger().fileLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}

	void Config::setStackLevel (LogLevel logLevel)
	{
		StackLogger &logger = getLogger();
		if (logger.maxStoredEvents != 0)
		{
			logger.stackLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
		}
	}

	void Config::setStackSize (unsigned int stackSize)
	{
		StackLogger &logger    = getLogger();
		logger.maxStoredEvents = stackSize;
		if (stackSize == 0)
		{
			logger.stackLevel = LogLevel::OFF;
		}
		logger.cleanExcedentEvents();
	}

	void Config::setOutFile (const std::string fileName)
	{
		StackLogger &logger = getLogger();

		logger.logFilePattern = fileName;

		// Force "reset" the file, and rotation config
		logger.hasRotation = true;
		logger.lastLogDate = std::chrono::year_month_day {};
	}

	void Config::setOutPath (const std::string filePath)
	{
		getLogger().logPath = filePath;
	}

	extern bool gMultiThreadSafe;
	extern bool isLoggerInitialized;

	void Config::setMultiThreadSafe (bool multiThreadSafe)
	{
		if (isLoggerInitialized && multiThreadSafe != gMultiThreadSafe)
		{
			std::string msg = "Cannot change the multi-thread safety after the logger has been initialized";
			getLogger().log (LogLevel::ERROR, msg);
		}
		else
		{
			gMultiThreadSafe = multiThreadSafe;
		}
	}

}    // namespace IgnacioPomar::Util::StreamLogger
