/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <format>
#include <chrono>
#include <iostream>

#include <filesystem>

#include "StreamLoggerConsts.h"
#include "StreamLoggerSettings.h"

#include "LoggerConsoleUtils.h"

#include "StackLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{

	namespace fs = std::filesystem;

	StackLogger::StackLogger()
	{
		this->setStackSize (DEFAULTS::STACK_SIZE);
		this->setStackLevel (DEFAULTS::STACK_LEVEL);
		this->setConsoleLevel (DEFAULTS::CONSOLE_LEVEL);
		this->setFileLevel (DEFAULTS::FILE_LEVEL);

		this->setFilePattern (DEFAULTS::FILE_NAME);
		this->setLogPath ("");
	}

	StackLogger::~StackLogger()
	{
		if (logfile.is_open())
		{
			this->logfile.flush();
			this->logfile.close();
		}
	}

	void StackLogger::sendEvents (LogEventsReceiver &receiver, LogLevel logLevel)
	{
		for (auto &event : events)
		{
			if (event.logLevel >= logLevel)
			{
				receiver.receive (event.date, event.event, event.logLevel);
			}
		}
	}

	void StackLogger::log (LogLevel logLevel, std::string &event)
	{
		if (!(logLevel >= consoleLevel || logLevel >= fileLevel || logLevel >= stackLevel))
		{
			return;
		}

		if (maxStoredEvents > 0 && logLevel >= stackLevel)
		{
			EventContainer &newEvent = events.emplace_back();
			fillEvent (newEvent, logLevel, event);
			this->sendToConsole (newEvent);
			this->sendToFile (newEvent);

			if (events.size() > maxStoredEvents)
			{
				// We have already inserted onne, so if we are over the limit, we need to remove the oldest
				events.pop_front();
			}
		}
		else
		{
			EventContainer tmpEvent;
			fillEvent (tmpEvent, logLevel, event);
			this->sendToConsole (tmpEvent);
			this->sendToFile (tmpEvent);
		}
	}

	void StackLogger::sendToConsole (EventContainer &event)
	{
		if (event.logLevel >= consoleLevel)
		{
			LogColor lc = levelColors [static_cast<int> (event.logLevel)];

			setConsoleColor (lc);
			std::clog << event.date << " [" << getLevelName (event.logLevel) << "]\t";
			std::clog << event.event << std::endl;
			resetConsoleColor();
		}
	}

	void StackLogger::sendToFile (EventContainer &event)
	{
		if (event.logLevel >= fileLevel)
		{
			// check rotation
			if (hasRotation)
			{
				auto dp  = floor<std::chrono::days> (event.timePoint);
				auto ymd = std::chrono::year_month_day {dp};

				if (ymd != lastLogDate)
				{
					lastLogDate = ymd;
					if (logfile.is_open())
					{
						this->logfile.close();
					}

					auto formattedDate = std::format ("{:04}-{:02}-{:02}", int (ymd.year()), unsigned (ymd.month()),
					                                  unsigned (ymd.day()));
					size_t pos         = logFilePattern.find ("%d");
					if (pos != std::string::npos)
					{
						this->logFilename = logFilePattern.replace (pos, 2, formattedDate);
					}
					else
					{
						hasRotation       = false;
						this->logFilename = logFilePattern;
					}
				}
			}

			if (!logfile.is_open())
			{
				fs::path filePath = fs::path (logPath) / this->logFilename;
				this->logfile.open (filePath, std::ios::out | std::ios::app);
				if (!this->logfile.is_open())
				{
					// Disable file logging
					this->fileLevel = LogLevel::OFF;

					// Generate event: unable to open log File
					std::string msg ("Unable to open log file: ");
					msg += filePath.string();
					this->log (LL::ERROR, msg);
				}
			}

			if (logfile.is_open())
			{
				this->logfile << event.date << " [" << getLevelName (event.logLevel) << "]\t";
				this->logfile << event.event << '\n';
			}
		}
	}

	void StackLogger::fillEvent (EventContainer &event, LogLevel logLevel, std::string &eventTxt)
	{
		event.event    = std::move (eventTxt);
		event.logLevel = logLevel;

		event.timePoint = std::chrono::system_clock::now();
		// #if __cplusplus >= 202002L
#if true
		event.date = format ("{}", event.timePoint);
#else
		auto in_time_t = std::chrono::system_clock::to_time_t (event.timePoint);
		struct tm buf;
		gmtime_s (&buf, &in_time_t);
		char str [100];
		strftime ((char *) str, sizeof (str), "%F %T UTC", &buf);

		event.date = str;
#endif
	}

	void StackLogger::setStackSize (unsigned int size)
	{
		this->maxStoredEvents = size;
		if (size == 0)
		{
			this->stackLevel = LogLevel::OFF;
		}
		// YAGNI: else {	this->stackLevel = storedStackLevel;		}

		while (events.size() > maxStoredEvents)
		{
			events.pop_front();
		}
	}

	void StackLogger::setConsoleLevel (LogLevel logLevel)
	{
		this->consoleLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}

	void StackLogger::setFileLevel (LogLevel logLevel)
	{
		this->fileLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}

	void StackLogger::setStackLevel (LogLevel logLevel)
	{
		if (this->maxStoredEvents != 0)
		{
			this->stackLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
		}
	}

	void StackLogger::setFilePattern (const std::string &pattern)
	{
		// Force "reset" the file, and rotation config
		this->hasRotation    = true;
		this->logFilePattern = pattern;
		this->lastLogDate    = std::chrono::year_month_day {};
	}

	void StackLogger::setLogPath (const std::string &path)
	{
		this->logPath = path;
	}

}    // namespace IgnacioPomar::Util::StreamLogger
