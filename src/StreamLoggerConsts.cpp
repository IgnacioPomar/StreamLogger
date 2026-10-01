/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "StreamLogger/StreamLoggerConsts.h"
#include "StreamLogger/StreamLogger.h"
#include "StackLogger.h"

#include <cstdio>

namespace IgnacioPomar::Util::StreamLogger
{
	const std::string logLevelNames [7] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF"};

	//--------------  Static Logger instances ----------------

	StaticLogger trace (LogLevel::TRACE);
	StaticLogger debug (LogLevel::DEBUG);
	StaticLogger info (LogLevel::INFO);
	StaticLogger warn (LogLevel::WARN);
	StaticLogger error (LogLevel::ERROR);
	StaticLogger fatal (LogLevel::FATAL);

	//--------------   Utility Functions ----------------
	const std::string &getLevelName (LogLevel logLevel)
	{
		int lvl = static_cast<int> (logLevel);
		return logLevelNames [(lvl < 6) ? lvl : 6];
	}

	namespace Internal
	{
		void reportError (const char *what) noexcept
		{
			// No allocations: it may be reporting a bad_alloc
			std::fputs ("StreamLogger: error while logging: ", stderr);
			std::fputs ((what != nullptr) ? what : "unknown", stderr);
			std::fputs ("\n", stderr);
		}
	}    // namespace Internal

	//-------------- Event retransmission ----------------

	void pullLogEvents (LogEventsSubscriber &subscriber, const LogLevel logLevel)
	{
		getLogger().sendEvents (subscriber, logLevel);
	}

	Subscription subscribe (LogEventsSubscriber &subscriber, const LogLevel logLevel)
	{
		return getLogger().subscribe (subscriber, logLevel);
	}

	void subscribePushEvents (LogEventsSubscriber &subscriber, const LogLevel logLevel)
	{
		getLogger().subscribePushEvents (subscriber, logLevel);
	}

	void unsubscribePushEvents (LogEventsSubscriber &subscriber)
	{
		getLogger().unsubscribePushEvents (subscriber);
	}

}    // namespace IgnacioPomar::Util::StreamLogger
