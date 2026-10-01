/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <string>

#include "StreamLogger/StreamLoggerConsts.h"
#include "StackLoggerConfig.h"
#include "LoggerConsoleUtils.h"
#include "StackLogger.h"
#include "StreamLogger/StreamLogger.h"

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
			getLogger().runLocked ([&] { getLogger().setStackSize (stackSize); });
		}

		void setOutFile (const std::string fileName)
		{
			getLogger().runLocked ([&] { getLogger().setOutFile (fileName); });
		}

		void setOutPath (const std::string filePath)
		{
			getLogger().runLocked ([&] { getLogger().setOutPath (filePath); });
		}
		void setLevelColor (LogLevel logLevel, LogColor logColor)
		{
			getLogger().runLocked ([&] { getLogger().setLevelColor (logLevel, logColor); });
		}

		void setColorMode (ColorMode colorMode)
		{
			getLogger().runLocked ([&] { getLogger().setColorMode (colorMode); });
		}

		void setConsoleLevel (LogLevel logLevel)
		{
			getLogger().runLocked ([&] { getLogger().setConsoleLevel (logLevel); });
		}

		void setFileLevel (LogLevel logLevel)
		{
			getLogger().runLocked ([&] { getLogger().setFileLevel (logLevel); });
		}

		void setStackLevel (LogLevel logLevel)
		{
			getLogger().runLocked ([&] { getLogger().setStackLevel (logLevel); });
		}

		void setFlushEvery (LogLevel logLevel, unsigned int events)
		{
			getLogger().runLocked ([&] { getLogger().setFlushEvery (logLevel, events); });
		}

		void setFlushInterval (std::chrono::milliseconds interval)
		{
			getLogger().runLocked ([&] { getLogger().setFlushInterval (interval); });
		}

		void flush ()
		{
			getLogger().runLocked ([&] { getLogger().flushFile(); });
		}
	};    // namespace Config

	//--------------  Configuration functions ----------------
	void StackLoggerConfig::setLevelColor (LogLevel logLevel, LogColor logColor)
	{
		int lvl = static_cast<int> (logLevel);
		if (lvl < 6)    // OFF has no color
		{
			this->levelColors [lvl] = logColor;
		}
	}

	void StackLoggerConfig::setColorMode (ColorMode colorMode)
	{
		switch (colorMode)
		{
		case ColorMode::ALWAYS: this->useColors = true; break;
		case ColorMode::NEVER: this->useColors = false; break;
		default: this->useColors = isColorConsole(); break;
		}
	}

	void StackLoggerConfig::setConsoleLevel (LogLevel logLevel)
	{
		// this->consoleLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
		this->consoleLevel = logLevel;
		this->setEffectiveLevel();
	}

	void StackLoggerConfig::setFileLevel (LogLevel logLevel)
	{
		// this->fileLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
		this->fileLevel = logLevel;
		this->setEffectiveLevel();
	}

	void StackLoggerConfig::setStackLevel (LogLevel logLevel)
	{
		if (this->maxStoredEvents != 0)
		{
			// this->stackLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
			this->stackLevel = logLevel;
		}
		else
		{
			this->stackLevel = LogLevel::OFF;
		}
		this->setEffectiveLevel();
	}

	void StackLoggerConfig::setFlushEvery (LogLevel logLevel, unsigned int events)
	{
		int lvl = static_cast<int> (logLevel);
		if (lvl < 6)    // OFF events are never written
		{
			this->flushEvery [lvl] = events;
		}
	}

	void StackLoggerConfig::setFlushInterval (std::chrono::milliseconds interval)
	{
		this->flushInterval = interval;
	}

	void StackLoggerConfig::resetSubscriberLevel()
	{
		this->subscriberLevel = LogLevel::OFF;
		this->setEffectiveLevel();
	}

	void StackLoggerConfig::addSubscriberLevel (LogLevel logLevel)
	{
		// LogLevel sbsLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;

		if (logLevel < this->subscriberLevel)
		{
			// this->subscriberLevel = sbsLevel;
			this->subscriberLevel = logLevel;
		}

		this->setEffectiveLevel();
	}

	void StackLoggerConfig::setEffectiveLevel()
	{
		LogLevel effLevel = this->stackLevel;
		if (this->consoleLevel < effLevel)
		{
			effLevel = this->consoleLevel;
		}
		if (this->fileLevel < effLevel)
		{
			effLevel = this->fileLevel;
		}
		if (this->subscriberLevel < effLevel)
		{
			effLevel = this->subscriberLevel;
		}
		if (this->stackLevel < effLevel)
		{
			effLevel = this->stackLevel;
		}
		this->effectiveLevel = effLevel;
	}

	StackLoggerConfig::StackLoggerConfig()
	{
		this->maxStoredEvents = DEFAULTS::STACK_SIZE;
		this->stackLevel      = DEFAULTS::STACK_LEVEL;
		this->consoleLevel    = DEFAULTS::CONSOLE_LEVEL;
		this->fileLevel       = DEFAULTS::FILE_LEVEL;

		this->resetSubscriberLevel();

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

		this->setColorMode (DEFAULTS::COLOR_MODE);

		for (int i = 0; i < 6; i++)
		{
			this->flushEvery [i]   = DEFAULTS::FLUSH_EVERY [i];
			this->pendingFlush [i] = 0;
		}
		this->flushInterval = DEFAULTS::FLUSH_INTERVAL;
		this->lastFlush     = TimePoint {};
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

		// Force reopening the file in the new path
		this->hasRotation = true;
		this->lastLogDate = std::chrono::year_month_day {};
	}

}    // namespace IgnacioPomar::Util::StreamLogger
