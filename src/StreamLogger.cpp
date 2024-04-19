/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <string>

#include "EventContainer.h"
#include "StreamLoggerConsts.h"
#include "StackLogger.h"
#include "StreamLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{

	//-------------- StaticLogger ----------------
	StaticLogger::StaticLogger (LogLevel level)
	    : level (level)
	{
	}

	void StaticLogger::log (std::string &message)
	{
		getLogger().log (level, message);
	}

	TimedEvent StaticLogger::startTimedEvent()
	{
		// Add new event in the stack logger (without the fill)
		return TimedEvent (getLogger().emplaceEvent (level));
	}

	//-------------- StaticLogger ----------------

	TimedEvent::~TimedEvent()
	{
		// The event has finised, we mark as finished, and reprocess it
		StackLogger &logger = getLogger();
		logger.fillElapsedTime (event);
		logger.processEvent (event);

		// YAGNI: If event inder the stackLevel, we should remove it from the stack
	}

	TimedEvent::TimedEvent (EventContainer &event)
	    : event (event)
	{
		event.eventType = EVENT_TYPE_TIMED_RUNNING;
	}

	void TimedEvent::log (std::string &message)
	{
		if (this->started)
		{
			// Call the log a second time means a second line of descriptions.
			// we simply add the message to the event
			event.event += message;
		}
		else
		{
			// In timed Events, log is in fact a "Start" event
			StackLogger &logger = getLogger();
			logger.fillEvent (event, message);
			logger.processEvent (event);
			this->started = true;
		}
	}

}    // namespace IgnacioPomar::Util::StreamLogger
