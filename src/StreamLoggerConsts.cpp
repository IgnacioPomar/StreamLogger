/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "StreamLoggerConsts.h"
#include "StreamLogger.h"
#include "StackLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{
	const std::string logLevelNames [6] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

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
		return logLevelNames [static_cast<int> (logLevel)];
	}

	//-------------- Event retransmission ----------------

	void pullLogEvents (LogEventsSubscriber &subscriber, const LogLevel logLevel)
	{
		getLogger().sendEvents (subscriber, logLevel);
	}

	LGGR_API void StreamLogger::subscribePushEvents (LogEventsSubscriber &subscriber, const LogLevel logLevel)
	{
		getLogger().subscribePushEvents (subscriber, logLevel);
	}

}    // namespace IgnacioPomar::Util::StreamLogger
