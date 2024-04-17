/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "StreamLoggerConsts.h"
#include "StreamLoggerSettings.h"

#include "StackLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{
	const std::string logLevelNames [6] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

	//--------------   Utility Functions ----------------
	const std::string &getLevelName (LogLevel logLevel)
	{
		return logLevelNames [static_cast<int> (logLevel)];
	}

	//-------------- Event retransmission ----------------

	void retrieveLogEvents (LogEventsReceiver &receiver, const LogLevel logLevel)
	{
		getLogger().sendEvents (receiver, logLevel);
	}

}    // namespace IgnacioPomar::Util::StreamLogger
