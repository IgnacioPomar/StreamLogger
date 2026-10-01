/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <chrono>
#include <cstdio>
#include <iostream>

#include <filesystem>

#include "StreamLogger/StreamLoggerConsts.h"

#include "LoggerConsoleUtils.h"

#include "StackLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{

	namespace fs = std::filesystem;

	namespace
	{
		// Same format with any compiler: 2024-04-17 15:28:07.215 UTC
		std::string formatTimestamp (TimePoint timePoint)
		{
			using namespace std::chrono;
			auto ms  = floor<milliseconds> (timePoint);
			auto day = floor<days> (ms);
			year_month_day ymd {day};
			hh_mm_ss hms {ms - day};

			char buf [48];
			std::snprintf (buf, sizeof (buf), "%04d-%02u-%02u %02d:%02d:%02d.%03d UTC", int (ymd.year()),
			               unsigned (ymd.month()), unsigned (ymd.day()), int (hms.hours().count()),
			               int (hms.minutes().count()), int (hms.seconds().count()), int (hms.subseconds().count()));
			return buf;
		}

		std::string formatDate (std::chrono::year_month_day ymd)
		{
			char buf [24];
			std::snprintf (buf, sizeof (buf), "%04d-%02u-%02u", int (ymd.year()), unsigned (ymd.month()),
			               unsigned (ymd.day()));
			return buf;
		}
	}    // namespace

	StackLogger::StackLogger() {}

	StackLogger::~StackLogger()
	{
		if (logfile.is_open())
		{
			this->logfile.flush();
			this->logfile.close();
		}
	}

	bool StackLogger::isEnabled (LogLevel logLevel) const noexcept
	{
		return logLevel >= this->effectiveLevel.load (std::memory_order_relaxed);
	}

	void StackLogger::sendEvents (LogEventsSubscriber &subscriber, LogLevel logLevel)
	{
		for (auto &event : events)
		{
			// A timed event not yet started has no data
			if (event.logLevel >= logLevel && !event.date.empty())
			{
				subscriber.onLogEvent (event.date, event.event, event.logLevel);
			}
		}
	}

	void StackLogger::subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel)
	{
		subscribers.emplace_back (receiver, logLevel);
		this->addSubscriberLevel (logLevel);
	}

	void StackLogger::unsubscribePushEvents (LogEventsSubscriber &receiver)
	{
		subscribers.remove_if ([&receiver] (const EventSubscriber &s) { return &s.subscriber == &receiver; });

		this->resetSubscriberLevel();
		for (auto &subscriber : subscribers)
		{
			this->addSubscriberLevel (subscriber.logLevel);
		}
	}

	void StackLogger::log (LogLevel logLevel, std::string &event)
	{
		if (logLevel < this->effectiveLevel)
		{
			return;
		}

		this->storeAndProcess (logLevel, event);
		this->cleanExcedentEvents();
	}

	void StackLogger::storeAndProcess (LogLevel logLevel, std::string &event)
	{
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
	}

	EventContainer *StackLogger::emplaceTimedEvent (LogLevel logLevel)
	{
		EventContainer &newEvent = events.emplace_back (logLevel);
		newEvent.eventType       = EVENT_TYPE_TIMED_RUNNING;
		return &newEvent;
	}

	void StackLogger::startTimedEvent (EventContainer &event, std::string &eventTxt)
	{
		this->fillEvent (event, eventTxt);
		this->processEvent (event);
	}

	void StackLogger::appendToTimedEvent (EventContainer &event, const std::string &eventTxt)
	{
		event.event += eventTxt;
	}

	void StackLogger::finishTimedEvent (EventContainer &event)
	{
		this->fillElapsedTime (event);
		this->processEvent (event);
		this->cleanExcedentEvents();
	}

	void StackLogger::discardTimedEvent (EventContainer &event)
	{
		this->events.remove_if ([&event] (const EventContainer &e) { return &e == &event; });
	}

	void StackLogger::runLocked (const std::function<void()> &action)
	{
		action();
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
			if (useColors)
			{
				setConsoleColor (levelColors [lvl]);
			}
			std::clog << event.date << " [" << getLevelName (event.logLevel) << "]\t";
			std::clog << event.event;
			if (useTimed)
			{
				std::clog << "\tDone in: " << event.usedTimeTxt;
			}
			std::clog << std::endl;
			if (useColors)
			{
				resetConsoleColor();
			}
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

					size_t pos = logFilePattern.find ("%d");
					if (pos != std::string::npos)
					{
						this->logFilename = logFilePattern;
						this->logFilename.replace (pos, 2, formatDate (ymd));
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
					// Not with log(): we are inside it (and, in the MT safe logger, with the mutex locked)
					std::string msg ("Unable to open log file: ");
					msg += filePath.string();
					this->storeAndProcess (LL::ERROR, msg);
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
				this->logfile << std::endl;    // A service may die without closing the file
			}
		}
	}

	void StackLogger::sendToSubscribers (EventContainer &event, bool useTimed)
	{
		// YAGNI: consider a thread for each subscriber if we are in MultiThreadSafe flavor
		// We would need a thread pool?
		if (event.logLevel >= this->subscriberLevel)
		{
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
	}

	void StackLogger::fillEvent (EventContainer &event, std::string &eventTxt)
	{
		event.event = std::move (eventTxt);

		event.timePoint = std::chrono::system_clock::now();
		event.date      = formatTimestamp (event.timePoint);
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
		if (milliseconds.count() > 0 || event.usedTimeTxt.empty())
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

	EventSubscriber::EventSubscriber (LogEventsSubscriber &subscriber, const LogLevel logLevel)
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

	void StackLoggerMTSafe::unsubscribePushEvents (LogEventsSubscriber &receiver)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		StackLogger::unsubscribePushEvents (receiver);
	}

	EventContainer *StackLoggerMTSafe::emplaceTimedEvent (LogLevel logLevel)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		return StackLogger::emplaceTimedEvent (logLevel);
	}

	void StackLoggerMTSafe::startTimedEvent (EventContainer &event, std::string &eventTxt)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		StackLogger::startTimedEvent (event, eventTxt);
	}

	void StackLoggerMTSafe::appendToTimedEvent (EventContainer &event, const std::string &eventTxt)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		StackLogger::appendToTimedEvent (event, eventTxt);
	}

	void StackLoggerMTSafe::finishTimedEvent (EventContainer &event)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		StackLogger::finishTimedEvent (event);
	}

	void StackLoggerMTSafe::discardTimedEvent (EventContainer &event)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		StackLogger::discardTimedEvent (event);
	}

	void StackLoggerMTSafe::runLocked (const std::function<void()> &action)
	{
		std::lock_guard<std::mutex> lock (this->mtx);
		action();
	}

}    // namespace IgnacioPomar::Util::StreamLogger
