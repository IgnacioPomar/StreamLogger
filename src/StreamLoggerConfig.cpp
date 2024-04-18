/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <string>

#include "StreamLoggerConsts.h"
#include "StreamLoggerSettings.h"
#include "StackLogger.h"
#include "StreamLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{
	//--------------  Configuration functions ----------------
	void Config::setLevelColor (LogLevel logLevel, LogColor logColor)
	{
		levelColors [static_cast<int> (logLevel)] = logColor;
	}

	void Config::setConsoleLevel (LogLevel logLevel)
	{
		getLogger().setConsoleLevel (logLevel);
	}
	void Config::setFileLevel (LogLevel logLevel)
	{
		getLogger().setFileLevel (logLevel);
	}
	void Config::setStackLevel (LogLevel logLevel)
	{
		getLogger().setStackLevel (logLevel);
	}

	void Config::setStackSize (unsigned int stackSize)
	{
		getLogger().setStackSize (stackSize);
	}

	void Config::setOutFile (const std::string fileName)
	{
		getLogger().setFilePattern (fileName);
	}

	void Config::setOutPath (const std::string filePath)
	{
		getLogger().setLogPath (filePath);
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
