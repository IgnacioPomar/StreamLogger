/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <string>

#include "EventContainer.h"
#include "StreamLogger/StreamLoggerConsts.h"
#include "StackLogger.h"
#include "StreamLogger/StreamLogger.h"

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

	bool StaticLogger::isEnabled() const
	{
		return getLogger().isEnabled (level);
	}

	TimedEvent StaticLogger::startTimedEvent()
	{
		if (!this->isEnabled())
		{
			return TimedEvent (nullptr);
		}
		// Add new event in the stack logger (without the fill), already marked as running
		return TimedEvent (getLogger().emplaceTimedEvent (level));
	}

	//-------------- TimedEvent ----------------

	TimedEvent::~TimedEvent()
	{
		if (this->event == nullptr)
		{
			return;
		}

		try
		{
			if (this->started)
			{
				// The event has finised, we mark as finished, and reprocess it
				getLogger().finishTimedEvent (*event);
			}
			else
			{
				getLogger().discardTimedEvent (*event);
			}
		}
		catch (const std::exception &e)
		{
			Internal::reportError (e.what());
		}
		catch (...)
		{
			Internal::reportError ("unknown exception finishing a timed event");
		}

		// YAGNI: If event inder the stackLevel, we should remove it from the stack
	}

	TimedEvent::TimedEvent (EventContainer *event) noexcept
	    : event (event)
	{
	}

	bool TimedEvent::isEnabled() const
	{
		return this->event != nullptr;
	}

	void TimedEvent::log (std::string &message)
	{
		if (this->event == nullptr)
		{
			return;
		}

		if (this->started)
		{
			// Call the log a second time means a second line of descriptions.
			// we simply add the message to the event
			getLogger().appendToTimedEvent (*event, message);
		}
		else
		{
			// In timed Events, log is in fact a "Start" event
			getLogger().startTimedEvent (*event, message);
			this->started = true;
		}
	}

}    // namespace IgnacioPomar::Util::StreamLogger
