/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef STACKLOGGER_H
#	define STACKLOGGER_H

#	include <list>
#	include <memory>
#	include <string>
#	include <fstream>
#	include <chrono>
#	include <vector>

#	include <functional>
#	include <mutex>

#	include "StreamLogger/StreamLoggerInterfaces.h"
#	include "StreamLogger/StreamLoggerConsts.h"
#	include "EventContainer.h"
#	include "StackLoggerConfig.h"

namespace IgnacioPomar::Util::StreamLogger
{
	class StackLogger;

	/**
	 * A push subscriber. Shared between the logger and the Subscription, so it survives
	 * while a callback is being dispatched outside the logger lock
	 */
	class SubscriberSlot
	{
		public:
			SubscriberSlot (StackLogger &owner, LogEventsSubscriber &subscriber, const LogLevel logLevel);

			StackLogger *owner;    // nullptr once the logger is destroyed
			LogEventsSubscriber &subscriber;
			const LogLevel logLevel;

			// Held while calling the subscriber: recursive, so the subscriber can unsubscribe inside its callback
			std::recursive_mutex callMtx;
			bool active = true;    // Protected by callMtx

			// Waits for a running callback, and avoids new ones
			void deactivate ();
	};

	using SubscriberList = std::vector<std::shared_ptr<SubscriberSlot>>;

	// Copy of an event to send to the subscribers after releasing the logger lock
	struct PushEvent
	{
			std::string date;
			std::string txt;
			LogLevel logLevel;
	};

	struct PendingDispatch
	{
			std::shared_ptr<const SubscriberList> subscribers;
			std::vector<PushEvent> events;
	};

	/**
	 * A logger wich stores the events in a stack
	 */
	class StackLogger : public StackLoggerConfig
	{
		private:
			std::list<EventContainer> events;

			// Copy on write: the dispatch uses a snapshot, without the lock
			std::shared_ptr<const SubscriberList> subscribers;

			std::ofstream logfile;

			// Prevent illegal usage
			StackLogger (const StackLogger &)            = delete;    // no copies
			StackLogger &operator= (const StackLogger &) = delete;    // no self-assignments
			StackLogger (StackLogger &&)                 = delete;    // no move constructor
			StackLogger &operator= (StackLogger &&)      = delete;    // no move assignments

			// The *Locked methods must be called with the lock acquired: the subscribers are only queued
			void sendToConsole (const EventContainer &event, const std::string &line);
			void sendToFile (EventContainer &event, const std::string &line, PendingDispatch &pending);
			void queueForSubscribers (const EventContainer &event, bool useTimed, PendingDispatch &pending);
			void processEvent (EventContainer &event, PendingDispatch &pending);
			void storeAndProcess (LogLevel logLevel, std::string &event, PendingDispatch &pending);
			void addSubscriber (const std::shared_ptr<SubscriberSlot> &slot);
			void removeSubscribers (const std::function<bool (const SubscriberSlot &)> &matches,
			                        SubscriberList &removed);

			// Called without the lock
			static void dispatch (PendingDispatch &pending);

		protected:
			void cleanExcedentEvents () override;

			// The MT safe version returns a locked mutex
			virtual std::unique_lock<std::mutex> acquire ();

		public:
			StackLogger();
			virtual ~StackLogger();

			void fillEvent (EventContainer &event, std::string &eventTxt);
			void fillElapsedTime (EventContainer &event);
			void flushFile ();

			// Without lock: the definitive check is done in log()
			bool isEnabled (LogLevel logLevel) const noexcept;

			// void delLogsOltherThan (int maxLogFileDays);

			// Entry points: they lock (in the MT safe version), and call the subscribers after unlocking
			void log (LogLevel logLevel, std::string &event);
			void sendEvents (LogEventsSubscriber &receiver, LogLevel logLevel);
			Subscription subscribe (LogEventsSubscriber &receiver, LogLevel logLevel);
			void subscribePushEvents (LogEventsSubscriber &receiver, LogLevel logLevel);
			void unsubscribePushEvents (LogEventsSubscriber &receiver);
			void unsubscribe (const std::shared_ptr<SubscriberSlot> &slot) noexcept;

			// The event is marked as running before releasing the lock: it can not be removed from the stack
			EventContainer *emplaceTimedEvent (LogLevel logLevel);
			void startTimedEvent (EventContainer &event, std::string &eventTxt);
			void appendToTimedEvent (EventContainer &event, const std::string &eventTxt);
			void finishTimedEvent (EventContainer &event);
			void discardTimedEvent (EventContainer &event);    // Never started: it is removed

			// Used to change the configuration
			void runLocked (const std::function<void()> &action);
	};

	class StackLoggerMTSafe : public StackLogger
	{
		private:
			std::mutex mtx;

			// Prevent illegal usage: this class is a singleton
			StackLoggerMTSafe (const StackLoggerMTSafe &)            = delete;    // no copies
			StackLoggerMTSafe &operator= (const StackLoggerMTSafe &) = delete;    // no self-assignments
			StackLoggerMTSafe (StackLoggerMTSafe &&)                 = delete;    // no move constructor
			StackLoggerMTSafe &operator= (StackLoggerMTSafe &&)      = delete;    // no move assignments

		protected:
			std::unique_lock<std::mutex> acquire () override;

		public:
			// Wee need the constructor to be public, as this class is a singleton
			StackLoggerMTSafe();
	};

	StackLogger &getLogger ();

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // STACKLOGGER_H
