

#include "StreamLoggerConsts.h"
#include "StreamLoggerSettings.h"

#include "StackLogger.h"

namespace StreamLogger
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

}    // namespace StreamLogger
