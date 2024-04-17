
#include <chrono>
#include <iostream>

#include "StreamLoggerConsts.h"
#include "StreamLoggerSettings.h"

#include "LoggerConsoleUtils.h"

#include "StackLogger.h"

namespace StreamLogger
{

	StackLogger::StackLogger()
	{
		this->setStackSize (stackSize);
	}

	void StackLogger::sendEvents (LogEventsReceiver &receiver, LogLevel logLevel)
	{
		for (auto &event : events)
		{
			if (event.logLevel >= logLevel)
			{
				receiver.receive (event.date, event.event, event.logLevel);
			}
		}
	}

	void StackLogger::log (LogLevel logLevel, std::string &event)
	{
		if (!(logLevel >= consoleLevel || logLevel >= fileLevel || logLevel >= stackLevel))
		{
			return;
		}

		if (logLevel >= stackLevel)
		{
			EventContainer &newEvent = events.emplace_back();
			fillEvent (newEvent, logLevel, event);
			this->sendToConsole (newEvent);
			this->sendToFile (newEvent);

			if (events.size() > maxStoredEvents)
			{
				// We have already inserted onne, so if we are over the limit, we need to remove the oldest
				events.pop_front();
			}
		}
		else
		{
			EventContainer tmpEvent;
			fillEvent (tmpEvent, logLevel, event);
			this->sendToConsole (tmpEvent);
			this->sendToFile (tmpEvent);
		}
	}

	void StackLogger::sendToConsole (EventContainer &event)
	{
		if (event.logLevel >= consoleLevel)
		{
			LogColor lc = levelColors [static_cast<int> (event.logLevel)];

			setConsoleColor (lc);
			std::cout << event.date << " [" << getLevelName (event.logLevel) << "]\t";
			std::cout << event.event << std::endl;
			resetConsoleColor();
		}
	}

	void StackLogger::sendToFile (EventContainer &event)
	{
		if (event.logLevel >= fileLevel)
		{
			// TODO:
		}
	}

	void StackLogger::fillEvent (EventContainer &event, LogLevel logLevel, std::string &eventTxt)
	{
		event.event    = std::move (eventTxt);
		event.logLevel = logLevel;

		auto now = std::chrono::system_clock::now();
		// #if __cplusplus >= 202002L
#if true
		event.date = format ("{}", now);
#else
		auto in_time_t = std::chrono::system_clock::to_time_t (now);
		struct tm buf;
		gmtime_s (&buf, &in_time_t);
		char str [100];
		strftime ((char *) str, sizeof (str), "%F %T UTC", &buf);

		event.date = str;
#endif
	}

	void StackLogger::setStackSize (unsigned int size)
	{
		this->maxStoredEvents = size;
		if (size == 0)
		{
		}

		// TODO: resize current events
	}
}    // namespace StreamLogger
