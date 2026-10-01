/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <chrono>
#include <cstdio>
#include <iostream>

#include <filesystem>

#include "StreamLogger/StreamLogger.h"
#include "StreamLogger/StreamLoggerConsts.h"

#include "LoggerConsoleUtils.h"

#include "StackLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{

	namespace fs = std::filesystem;

	namespace
	{
		// > 0 while this thread is inside a subscriber callback: its events are not pushed (avoids loops)
		thread_local int callbackDepth = 0;

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

	//-------------- SubscriberSlot / Subscription ----------------

	SubscriberSlot::SubscriberSlot (StackLogger &owner, LogEventsSubscriber &subscriber, const LogLevel logLevel)
	    : owner (&owner)
	    , subscriber (subscriber)
	    , logLevel (logLevel)
	{
	}

	void SubscriberSlot::deactivate()
	{
		std::lock_guard<std::recursive_mutex> lock (this->callMtx);
		this->active = false;
	}

	Subscription::Subscription (std::shared_ptr<SubscriberSlot> slot) noexcept
	    : slot (std::move (slot))
	{
	}

	Subscription::Subscription (Subscription &&other) noexcept = default;

	Subscription &Subscription::operator= (Subscription &&other) noexcept
	{
		if (this != &other)
		{
			this->reset();
			this->slot = std::move (other.slot);
		}
		return *this;
	}

	Subscription::~Subscription()
	{
		this->reset();
	}

	void Subscription::reset() noexcept
	{
		if (!this->slot)
		{
			return;
		}
		std::shared_ptr<SubscriberSlot> oldSlot = std::move (this->slot);
		if (oldSlot->owner != nullptr)
		{
			oldSlot->owner->unsubscribe (oldSlot);
		}
		else
		{
			oldSlot->deactivate();
		}
	}

	Subscription::operator bool() const noexcept
	{
		return static_cast<bool> (this->slot);
	}

	//-------------- StackLogger ----------------

	StackLogger::StackLogger()
	    : subscribers (std::make_shared<const SubscriberList>())
	{
	}

	StackLogger::~StackLogger()
	{
		// The remaining Subscriptions must not use this logger
		for (auto &slot : *this->subscribers)
		{
			slot->owner = nullptr;
		}

		if (logfile.is_open())
		{
			this->logfile.flush();
			this->logfile.close();
		}
	}

	std::unique_lock<std::mutex> StackLogger::acquire()
	{
		return {};
	}

	bool StackLogger::isEnabled (LogLevel logLevel) const noexcept
	{
		return logLevel >= this->effectiveLevel.load (std::memory_order_relaxed);
	}

	void StackLogger::sendEvents (LogEventsSubscriber &subscriber, LogLevel logLevel)
	{
		// Copy, to call the subscriber without the lock
		std::vector<PushEvent> copy;
		{
			auto lock = this->acquire();
			for (auto &event : events)
			{
				// A timed event not yet started has no data
				if (event.logLevel >= logLevel && !event.date.empty())
				{
					copy.push_back ({event.date, event.event, event.logLevel});
				}
			}
		}

		for (auto &event : copy)
		{
			subscriber.onLogEvent (event.date, std::move (event.txt), event.logLevel);
		}
	}

	Subscription StackLogger::subscribe (LogEventsSubscriber &receiver, LogLevel logLevel)
	{
		auto slot = std::make_shared<SubscriberSlot> (*this, receiver, logLevel);
		{
			auto lock = this->acquire();
			this->addSubscriber (slot);
		}
		return Subscription (slot);
	}

	void StackLogger::subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel)
	{
		auto slot = std::make_shared<SubscriberSlot> (*this, receiver, logLevel);
		auto lock = this->acquire();
		this->addSubscriber (slot);
	}

	void StackLogger::unsubscribePushEvents (LogEventsSubscriber &receiver)
	{
		SubscriberList removed;
		{
			auto lock = this->acquire();
			this->removeSubscribers ([&receiver] (const SubscriberSlot &s) { return &s.subscriber == &receiver; },
			                         removed);
		}

		// Without the logger lock: a running callback may be logging
		for (auto &slot : removed)
		{
			slot->deactivate();
		}
	}

	void StackLogger::unsubscribe (const std::shared_ptr<SubscriberSlot> &slot) noexcept
	{
		// First, without the logger lock: wait for a running callback (it may be logging)
		slot->deactivate();

		try
		{
			SubscriberList removed;
			auto lock = this->acquire();
			this->removeSubscribers ([&slot] (const SubscriberSlot &s) { return &s == slot.get(); }, removed);
		}
		catch (const std::exception &e)
		{
			// The slot is already inactive: it is only a leak
			Internal::reportError (e.what());
		}
	}

	void StackLogger::addSubscriber (const std::shared_ptr<SubscriberSlot> &slot)
	{
		auto newList = std::make_shared<SubscriberList> (*this->subscribers);
		newList->push_back (slot);
		this->subscribers = std::move (newList);
		this->addSubscriberLevel (slot->logLevel);
	}

	void StackLogger::removeSubscribers (const std::function<bool (const SubscriberSlot &)> &matches,
	                                     SubscriberList &removed)
	{
		auto newList = std::make_shared<SubscriberList>();
		for (auto &slot : *this->subscribers)
		{
			(matches (*slot) ? removed : *newList).push_back (slot);
		}
		this->subscribers = std::move (newList);

		this->resetSubscriberLevel();
		for (auto &slot : *this->subscribers)
		{
			this->addSubscriberLevel (slot->logLevel);
		}
	}

	void StackLogger::log (LogLevel logLevel, std::string &event)
	{
		PendingDispatch pending;
		{
			auto lock = this->acquire();
			if (logLevel < this->effectiveLevel)
			{
				return;
			}

			this->storeAndProcess (logLevel, event, pending);
			this->cleanExcedentEvents();
		}
		dispatch (pending);
	}

	void StackLogger::storeAndProcess (LogLevel logLevel, std::string &event, PendingDispatch &pending)
	{
		if (maxStoredEvents > 0 && logLevel >= stackLevel)
		{
			EventContainer &newEvent = this->events.emplace_back (logLevel);
			fillEvent (newEvent, event);
			this->processEvent (newEvent, pending);
		}
		else
		{
			EventContainer tmpEvent (logLevel);
			fillEvent (tmpEvent, event);
			this->processEvent (tmpEvent, pending);
		}
	}

	EventContainer *StackLogger::emplaceTimedEvent (LogLevel logLevel)
	{
		auto lock                = this->acquire();
		EventContainer &newEvent = events.emplace_back (logLevel);
		newEvent.eventType       = EVENT_TYPE_TIMED_RUNNING;
		return &newEvent;
	}

	void StackLogger::startTimedEvent (EventContainer &event, std::string &eventTxt)
	{
		PendingDispatch pending;
		{
			auto lock = this->acquire();
			this->fillEvent (event, eventTxt);
			this->processEvent (event, pending);
		}
		dispatch (pending);
	}

	void StackLogger::appendToTimedEvent (EventContainer &event, const std::string &eventTxt)
	{
		auto lock = this->acquire();
		event.event += eventTxt;
	}

	void StackLogger::finishTimedEvent (EventContainer &event)
	{
		PendingDispatch pending;
		{
			auto lock = this->acquire();
			this->fillElapsedTime (event);
			this->processEvent (event, pending);
			this->cleanExcedentEvents();
		}
		dispatch (pending);
	}

	void StackLogger::discardTimedEvent (EventContainer &event)
	{
		auto lock = this->acquire();
		this->events.remove_if ([&event] (const EventContainer &e) { return &e == &event; });
	}

	void StackLogger::runLocked (const std::function<void()> &action)
	{
		auto lock = this->acquire();
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

	void StackLogger::sendToFile (EventContainer &event, bool useTimed, PendingDispatch &pending)
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
					this->storeAndProcess (LL::ERROR, msg, pending);
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

	void StackLogger::queueForSubscribers (const EventContainer &event, bool useTimed, PendingDispatch &pending)
	{
		if (event.logLevel < this->subscriberLevel || callbackDepth > 0)
		{
			return;
		}
		if (!pending.subscribers)
		{
			pending.subscribers = this->subscribers;
		}
		if (useTimed)
		{
			pending.events.push_back ({event.date, event.event + "\tDone in: " + event.usedTimeTxt, event.logLevel});
		}
		else
		{
			pending.events.push_back ({event.date, event.event, event.logLevel});
		}
	}

	void StackLogger::dispatch (PendingDispatch &pending)
	{
		// YAGNI: consider a thread for each subscriber if we are in MultiThreadSafe flavor
		for (auto &event : pending.events)
		{
			for (auto &slot : *pending.subscribers)
			{
				if (event.logLevel < slot->logLevel)
				{
					continue;
				}

				std::lock_guard<std::recursive_mutex> lock (slot->callMtx);
				if (!slot->active)
				{
					continue;
				}

				++callbackDepth;
				try
				{
					slot->subscriber.onLogEvent (event.date, event.txt, event.logLevel);
				}
				catch (const std::exception &e)
				{
					Internal::reportError (e.what());
				}
				catch (...)
				{
					Internal::reportError ("unknown exception in a subscriber");
				}
				--callbackDepth;
			}
		}
	}

	void StackLogger::fillEvent (EventContainer &event, std::string &eventTxt)
	{
		event.event     = std::move (eventTxt);
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

	void StackLogger::processEvent (EventContainer &event, PendingDispatch &pending)
	{
		// Only with finished Event timed events
		bool useTimed = EVENT_TYPE_TIMED_FINISHED == event.eventType;

		this->sendToConsole (event, useTimed);
		this->sendToFile (event, useTimed, pending);
		this->queueForSubscribers (event, useTimed, pending);
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

	// ------------------- StackLoggerMTSafe -------------------
	// This class adds mutex protection to StackLogger

	StackLoggerMTSafe::StackLoggerMTSafe()
	    : StackLogger()
	{
	}

	std::unique_lock<std::mutex> StackLoggerMTSafe::acquire()
	{
		return std::unique_lock<std::mutex> (this->mtx);
	}

}    // namespace IgnacioPomar::Util::StreamLogger
