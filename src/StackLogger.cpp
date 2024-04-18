/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
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
		this->setDefaultValues();
	}

	void StreamLogger::StackLogger::setDefaultValues()
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

	StackLogger::~StackLogger()
	{
		if (logfile.is_open())
		{
			this->logfile.flush();
			this->logfile.close();
		}
	}

	void StackLogger::sendEvents (LogEventsSubscriber &subscriber, LogLevel logLevel)
	{
		for (auto &event : events)
		{
			if (event.logLevel >= logLevel)
			{
				subscriber.onLogEvent (event.date, event.event, event.logLevel);
			}
		}
	}

	void StackLogger::subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel)
	{
		subscribers.emplace_back (receiver, logLevel);
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
			this->sendToSubscribers (newEvent);

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
			this->sendToSubscribers (tmpEvent);
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

	void StackLogger::sendToSubscribers (EventContainer &event)
	{
		for (auto &subscriber : subscribers)
		{
			if (event.logLevel >= subscriber.logLevel)
			{
				subscriber.subscriber.onLogEvent (event.date, event.event, event.logLevel);
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

	void StackLogger::cleanExcedentEvents()
	{
		while (events.size() > maxStoredEvents)
		{
			events.pop_front();
		}
	}

	EventSubscriber::EventSubscriber (const LogEventsSubscriber &subscriber, const LogLevel logLevel)
	    : subscriber (subscriber)
	    , logLevel (logLevel)
	{
	}

	// ------------------- StackLoggerMTSafe -------------------
	// This class is a wrapper for StackLogger, adding mutex protection

	StackLoggerMTSafe::StackLoggerMTSafe()
	    : StackLogger()
	{
	}

	void StackLoggerMTSafe::log (LogLevel logLevel, std::string &event)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		StackLogger::log (logLevel, event);
	}

	void StackLoggerMTSafe::sendEvents (LogEventsSubscriber &receiver, LogLevel logLevel)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		StackLogger::sendEvents (receiver, logLevel);
	}

	void StackLoggerMTSafe::subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel)
	{
		// do we need to lock the mutex here? It'll happens at the begining of the program, so it should be safe
		std::lock_guard<std::mutex> lock (this->mtx);
		StackLogger::subscribePushEvents (receiver, logLevel);
	}

}    // namespace IgnacioPomar::Util::StreamLogger
