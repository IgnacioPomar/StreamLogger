/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef __STREAM_LOGGER_INTERFACES_H
#	define __STREAM_LOGGER_INTERFACES_H

#	include <memory>
#	include <string>

#	include "StreamLoggerConsts.h"

namespace IgnacioPomar::Util::StreamLogger
{
	// ----------------------   Util functions -------------------------------------
	const std::string &getLevelName (const LogLevel logLevel);

	//  We cant use std::string_view because in multi-threading, the subyacent string could be deleted
	//--- Retrieve the generated events ---
	class LogEventsSubscriber
	{
		public:
			virtual ~LogEventsSubscriber() = default;
			virtual void onLogEvent (const std::string &date, const std::string logTxt, const LogLevel logLevel) = 0;
	};

	class SubscriberSlot;

	/**
	 * RAII push subscription: on destruction (or reset) it unsubscribes, waiting for a running callback to finish.
	 * Once it is destroyed, the subscriber will not be called again, so the subscriber can be safely destroyed.
	 * If it is a member of the subscriber itself, declare it the last one: it must be destroyed first.
	 *
	 * The callbacks are called without the logger locked, so they can log, but:
	 *  - The events logged inside a callback are not pushed to the subscribers (avoids infinite loops)
	 *  - A subscriber is never called concurrently by the same logger (even if subscribed twice), but the events of
	 *    different threads may arrive in a different order than the one in the stack
	 *  - An exception thrown by a callback is reported in stderr, and ignored
	 */
	class Subscription
	{
		public:
			Subscription() noexcept = default;
			explicit Subscription (std::shared_ptr<SubscriberSlot> slot) noexcept;
			Subscription (Subscription &&other) noexcept;
			Subscription &operator= (Subscription &&other) noexcept;
			Subscription (const Subscription &)            = delete;
			Subscription &operator= (const Subscription &) = delete;
			~Subscription();

			void reset () noexcept;
			explicit operator bool () const noexcept;

		private:
			std::shared_ptr<SubscriberSlot> slot;
	};

	void pullLogEvents (LogEventsSubscriber &subscriber, const LogLevel logLevel);
	[[nodiscard]] Subscription subscribe (LogEventsSubscriber &subscriber, const LogLevel logLevel);

	[[deprecated ("Use subscribe(): the Subscription unsubscribes on destruction")]] void subscribePushEvents (
	    LogEventsSubscriber &subscriber, const LogLevel logLevel);
	// Must be called before the subscriber is destroyed
	[[deprecated ("Use subscribe(): the Subscription unsubscribes on destruction")]] void
	    unsubscribePushEvents (LogEventsSubscriber &subscriber);

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // __STREAM_LOGGER_INTERFACES_H
