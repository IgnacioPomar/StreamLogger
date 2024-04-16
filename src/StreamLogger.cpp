/*********************************************************************************************
 *	Name		: StreamLogger.h
 *	Description	: Stream logger Implementation
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

#include "StreamLogger.h"

namespace StreamLogger
{
	void setConsoleColor (LogColor color);
	void resetConsoleColor ();

	//--------------  Static values ----------------
	const std::string logLevelNames [6] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
	BaseStreamLogger trace (LogLevel::TRACE);
	BaseStreamLogger debug (LogLevel::DEBUG);
	BaseStreamLogger info (LogLevel::INFO);
	BaseStreamLogger warn (LogLevel::WARN);
	BaseStreamLogger error (LogLevel::ERROR);
	BaseStreamLogger fatal (LogLevel::FATAL);

	//--------------  Configuration Vars ----------------
	bool rotateByDay = true;
	std::string logPath;
	std::string logName = DEFAULTS::FILE_NAME;

	int stackSize = DEFAULTS::STACK_SIZE;

	LogColor levelColors [6] = {DEFAULTS::COLOR_TRACE, DEFAULTS::COLOR_DEBUG, DEFAULTS::COLOR_INFO,
	                            DEFAULTS::COLOR_WARN,  DEFAULTS::COLOR_ERROR, DEFAULTS::COLOR_FATAL};

	LogLevel consoleLevel = DEFAULTS::CONSOLE_LEVEL;
	LogLevel fileLevel    = DEFAULTS::FILE_LEVEL;
	LogLevel stackLevel   = DEFAULTS::STACK_LEVEL;

	//--------------  Configuration functions ----------------
	void Config::setLevelColor (LogLevel logLevel, LogColor logColor)
	{
		levelColors [static_cast<int> (logLevel)] = logColor;
	}

	void Config::setConsoleLevel (LogLevel logLevel)
	{
		consoleLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}
	void Config::setFileLevel (LogLevel logLevel)
	{
		fileLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}
	void Config::setStackLevel (LogLevel logLevel)
	{
		stackLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}

	//--------------   Utility Functions ----------------
	const std::string &getLevelName (LogLevel logLevel)
	{
		return logLevelNames [static_cast<int> (logLevel)];
	}

	//--------------  Real function class ----------------
	/**
	 * Contain the info for a single event
	 */
	class EventContainer
	{
		public:
			// Storing the date as a string to avoid reformating each time we send it
			//  Or, should we store it as chrono::time_point?
			std::string date;
			std::string event;
			LogLevel logLevel;
	};

	/**
	 * A logger wich stores the events in a stack
	 */
	class StackLogger
	{
		private:
			std::list<EventContainer> events;

			std::ofstream logfile;

			unsigned int maxStoredEvents;

			// Prevent illegal usage
			StackLogger (const StackLogger &)            = delete;    // no copies
			StackLogger &operator= (const StackLogger &) = delete;    // no self-assignments
			StackLogger (StackLogger &&)                 = delete;    // no move constructor
			StackLogger &operator= (StackLogger &&)      = delete;    // no move assignments

			void sendToConsole (EventContainer &event);
			void sendToFile (EventContainer &event);
			void fillEvent (EventContainer &event, LogLevel logLevel, std::string &eventTxt);

		public:
			StackLogger();
			//~StackLogger ();
			// void delLogsOltherThan (int maxLogFileDays);

			void log (LogLevel logLevel, std::string &event);
			void sendEvents (LogEventsReceiver &receiver, LogLevel logLevel);
	};

	StackLogger stackLogger;

	StackLogger::StackLogger()
	    : maxStoredEvents (stackSize)
	{
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

	//-------------- LogMessageBuilder ----------------
	/**
	 * Constructor
	 */
	LogMessageBuilder::LogMessageBuilder (BaseStreamLogger &logger)
	    : logger (logger)
	{
	}

	/**
	 * Move constructor
	 */
	LogMessageBuilder::LogMessageBuilder (LogMessageBuilder &&other) noexcept
	    : logger (other.logger)
	    , message (std::move (other.message))
	{
	}

	/**
	 * Destructor, log the message
	 */
	LogMessageBuilder::~LogMessageBuilder()
	{
		// If using string stream
		// logger.log (stream.str ());

		// As long as message moves to child, we only send it if it is not empty
		if (!message.empty())
		{
			logger.log (message);
		}
	}

	LogMessageBuilder &LogMessageBuilder::operator<< (const std::string &s)
	{
		this->message.append (s);
		return *this;
	}
	LogMessageBuilder &LogMessageBuilder::operator<< (int value)
	{
		this->message.append (std::to_string (value));
		return *this;
	}
	LogMessageBuilder &LogMessageBuilder::operator<< (double value)
	{
		this->message.append (std::to_string (value));
		return *this;
	}

	//-------------- BaseStreamLogger ----------------
	BaseStreamLogger::BaseStreamLogger (LogLevel level)
	    : level (level)
	{
	}

	void BaseStreamLogger::log (std::string &message)
	{
		stackLogger.log (level, message);
	}

	LogMessageBuilder BaseStreamLogger::operator<< (const std::string &value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}
	LogMessageBuilder BaseStreamLogger::operator<< (int value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}
	LogMessageBuilder BaseStreamLogger::operator<< (double value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}

	//-------------- Event retransmission ----------------

	void retrieveLogEvents (LogEventsReceiver &receiver, const LogLevel logLevel)
	{
		stackLogger.sendEvents (receiver, logLevel);
	}

}    // namespace StreamLogger
