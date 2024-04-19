/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <string>

#include "StreamLoggerConsts.h"
#include "StackLoggerConfig.h"
#include "StackLogger.h"
#include "StreamLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{
	extern bool gMultiThreadSafe;
	extern bool isLoggerInitialized;
	namespace Config
	{
		void setMultiThreadSafe (bool multiThreadSafe)
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

		void setStackSize (unsigned int stackSize)
		{
			getLogger().setStackSize (stackSize);
		}

		void setOutFile (const std::string fileName)
		{
			getLogger().setOutFile (fileName);
		}

		void setOutPath (const std::string filePath)
		{
			getLogger().setOutPath (filePath);
		}
		void setLevelColor (LogLevel logLevel, LogColor logColor)
		{
			getLogger().setLevelColor (logLevel, logColor);
		}

		void setConsoleLevel (LogLevel logLevel)
		{
			getLogger().setConsoleLevel (logLevel);
		}

		void setFileLevel (LogLevel logLevel)
		{
			getLogger().setFileLevel (logLevel);
		}

		void setStackLevel (LogLevel logLevel)
		{
			getLogger().setStackLevel (logLevel);
		}
	};    // namespace Config

	//--------------  Configuration functions ----------------
	void StackLoggerConfig::setLevelColor (LogLevel logLevel, LogColor logColor)
	{
		this->levelColors [static_cast<int> (logLevel)] = logColor;
	}

	void StackLoggerConfig::setConsoleLevel (LogLevel logLevel)
	{
		this->consoleLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}

	void StackLoggerConfig::setFileLevel (LogLevel logLevel)
	{
		this->fileLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}

	void StackLoggerConfig::setStackLevel (LogLevel logLevel)
	{
		if (this->maxStoredEvents != 0)
		{
			this->stackLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
		}
	}

	StackLoggerConfig::StackLoggerConfig()
	{
		this->maxStoredEvents = DEFAULTS::STACK_SIZE;
		this->stackLevel      = DEFAULTS::STACK_LEVEL;
		this->consoleLevel    = DEFAULTS::CONSOLE_LEVEL;
		this->fileLevel       = DEFAULTS::FILE_LEVEL;

		this->hasRotation    = true;
		this->logFilePattern = DEFAULTS::FILE_NAME;
		this->lastLogDate    = std::chrono::year_month_day {};

		this->logPath = "";

		this->levelColors [0] = DEFAULTS::COLOR_TRACE;
		this->levelColors [1] = DEFAULTS::COLOR_DEBUG;
		this->levelColors [2] = DEFAULTS::COLOR_INFO;
		this->levelColors [3] = DEFAULTS::COLOR_WARN;
		this->levelColors [4] = DEFAULTS::COLOR_ERROR;
		this->levelColors [5] = DEFAULTS::COLOR_FATAL;
	}

	void StackLoggerConfig::setStackSize (unsigned int stackSize)
	{
		this->maxStoredEvents = stackSize;
		if (stackSize == 0)
		{
			this->stackLevel = LogLevel::OFF;
		}
		this->cleanExcedentEvents();
	}

	void StackLoggerConfig::setOutFile (const std::string fileName)
	{
		this->logFilePattern = fileName;

		// Force "reset" the file, and rotation config
		this->hasRotation = true;
		this->lastLogDate = std::chrono::year_month_day {};
	}

	void StackLoggerConfig::setOutPath (const std::string filePath)
	{
		this->logPath = filePath;
	}

}    // namespace IgnacioPomar::Util::StreamLogger
