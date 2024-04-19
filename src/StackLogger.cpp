/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#if __has_include(<format>)
#	include <format>
#endif
#include <chrono>
#include <iostream>

#include <filesystem>

#include "StreamLoggerConsts.h"

#include "LoggerConsoleUtils.h"

#include "StackLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{

	namespace fs = std::filesystem;

	StackLogger::StackLogger() {}

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
			EventContainer &newEvent = this->events.emplace_back (logLevel);
			fillEvent (newEvent, event);
			this->processEvent (newEvent);
		}
		else
		{
			EventContainer tmpEvent (logLevel);
			fillEvent (tmpEvent, event);
			this->processEvent (tmpEvent);
		}

		this->cleanExcedentEvents();
	}

	EventContainer &StackLogger::emplaceEvent (LogLevel logLevel)
	{
		return events.emplace_back (logLevel);
	}

	void StackLogger::sendToConsole (EventContainer &event, bool useTimed)
	{
		if (event.logLevel >= consoleLevel)
		{
			int lvl = static_cast<int> (event.logLevel);
			if (lvl > 5)
			{
				lvl = static_cast<int> (LogLevel::FATAL);
			}
			LogColor lc = levelColors [lvl];

			setConsoleColor (lc);
			std::clog << event.date << " [" << getLevelName (event.logLevel) << "]\t";
			std::clog << event.event;
			if (useTimed)
			{
				std::clog << "\tDone in: " << event.usedTimeTxt;
			}
			std::clog << std::endl;
			resetConsoleColor();
		}
	}

	void StackLogger::sendToFile (EventContainer &event, bool useTimed)
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

#if __has_include(<format>)
					auto formattedDate = std::format ("{:04}-{:02}-{:02}", int (ymd.year()), unsigned (ymd.month()),
					                                  unsigned (ymd.day()));
#else
					std::ostringstream oss;
					oss << std::setw (4) << std::setfill ('0') << int (ymd.year()) << "-";
					oss << std::setw (2) << std::setfill ('0') << unsigned (ymd.month()) << "-";
					oss << std::setw (2) << std::setfill ('0') << unsigned (ymd.day());

					std::string formattedDate = oss.str();
#endif
					size_t pos = logFilePattern.find ("%d");
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
				this->logfile << event.event;
				if (useTimed)
				{
					this->logfile << "\tDone in: " << event.usedTimeTxt;
				}
				this->logfile << '\n';
			}
		}
	}

	void StackLogger::sendToSubscribers (EventContainer &event, bool useTimed)
	{
		// YAGNI: consider a thread for each subscriber if we are in MultiThreadSafe flavor
		// We would need a thread pool?
		for (auto &subscriber : subscribers)
		{
			if (event.logLevel >= subscriber.logLevel)
			{
				if (useTimed)
				{
					subscriber.subscriber.onLogEvent (event.date, event.event + "\tDone in: " + event.usedTimeTxt,
					                                  event.logLevel);
				}
				else
				{
					subscriber.subscriber.onLogEvent (event.date, event.event, event.logLevel);
				}
			}
		}
	}

	void StackLogger::fillEvent (EventContainer &event, std::string &eventTxt)
	{
		event.event = std::move (eventTxt);

		event.timePoint = std::chrono::system_clock::now();
#if __has_include(<format>)

		event.date = format ("{}", event.timePoint);
#else
		auto in_time_t = std::chrono::system_clock::to_time_t (event.timePoint);
		struct tm buf;
		gmtime_r (&in_time_t, &buf);
		char str [100];
		strftime ((char *) str, sizeof (str), "%F %T UTC", &buf);

		event.date = str;
#endif
	}

	void StackLogger::fillElapsedTime (EventContainer &event)
	{
		event.endTimePoint = std::chrono::system_clock::now();
		event.eventType    = EVENT_TYPE_TIMED_FINISHED;

		// Compute the duration in milliseconds
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds> (event.endTimePoint - event.timePoint);

		// Extract time components
		auto hours = std::chrono::duration_cast<std::chrono::hours> (duration);
		duration -= hours;
		auto minutes = std::chrono::duration_cast<std::chrono::minutes> (duration);
		duration -= minutes;
		auto seconds = std::chrono::duration_cast<std::chrono::seconds> (duration);
		duration -= seconds;
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (duration);

		// Format the output string
		event.usedTimeTxt = "";
		if (hours.count() > 0)
		{
			event.usedTimeTxt += std::to_string (hours.count()) + "h ";
		}
		if (minutes.count() > 0 || !event.usedTimeTxt.empty())
		{
			event.usedTimeTxt += std::to_string (minutes.count()) + "' ";
		}
		if (seconds.count() > 0 || !event.usedTimeTxt.empty())
		{
			event.usedTimeTxt += std::to_string (seconds.count()) + "\" ";
		}
		if (milliseconds.count() > 0)
		{
			event.usedTimeTxt += std::to_string (milliseconds.count()) + "ms";
		}
	}

	void StackLogger::processEvent (EventContainer &event)
	{
		// Only with finished Event timed events
		bool useTimed = EVENT_TYPE_TIMED_FINISHED == event.eventType;

		this->sendToConsole (event, useTimed);
		this->sendToFile (event, useTimed);
		this->sendToSubscribers (event, useTimed);
	}

	void StackLogger::cleanExcedentEvents()
	{
		// Delete excedents, skiping the running timed events
		// See maxStoredEvents notes in the header
		auto it = events.begin();
		while (events.size() > maxStoredEvents && it != events.end())
		{
			if ((*it).eventType & EVENT_TYPE_RUNNING)
			{
				// Skip the event if it matches the EVENT_TYPE_RUNNING mask
				++it;
			}
			else
			{
				// Erase the event from the deque if it doesn't match the mask
				it = events.erase (it);    // This safely increments the iterator
			}
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

	EventContainer &StackLoggerMTSafe::emplaceEvent (LogLevel logLevel)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		return StackLogger::emplaceEvent (logLevel);
	}

}    // namespace IgnacioPomar::Util::StreamLogger
