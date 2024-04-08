/*********************************************************************************************
*	Name		: StreamLogger.h
*	Description	: Stream logger Implementation
*	Copyright	(C) 2024  Ignacio Pomar Ballestero
********************************************************************************************/


#include <filesystem>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <list>

#include "StreamLogger.h"

namespace StreamLogger
{
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


	LogColor levelColors [6] = {DEFAULTS::COLOR_TRACE, DEFAULTS::COLOR_DEBUG, DEFAULTS::COLOR_INFO, DEFAULTS::COLOR_WARN, DEFAULTS::COLOR_ERROR, DEFAULTS::COLOR_FATAL};

	LogLevel consoleLevel = DEFAULTS::CONSOLE_LEVEL;
	LogLevel fileLevel = DEFAULTS::FILE_LEVEL;
	LogLevel stackLevel = DEFAULTS::STACK_LEVEL;


	//--------------  Configuration functions ----------------
	void Config::setLevelColor (LogLevel logLevel, LogColor logColor)
	{
		levelColors [static_cast<int>(logLevel)] = logColor;
	}


	void Config::setConsoleLevel (BaseStreamLogger logLevel)
	{
		setConsoleLevel (logLevel.level);
	}
	void Config::setFileLevel (BaseStreamLogger logLevel)
	{
		setFileLevel (logLevel.level);
	}
	void Config::setStackLevel (BaseStreamLogger logLevel)
	{
		setStackLevel (logLevel.level);
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
	const std::string& getLevelName (LogLevel logLevel)
	{
		return logLevelNames [static_cast<int>(logLevel)];
	}

	//--------------  Real function class ----------------
	/**
	* Contain the info for a single event
	*/
	class EventContainer
	{
	public:
		//Storing the date as a string to avoid reformating each time we send it
		// Or, should we store it as chrono::time_point?
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


		//Prevent illegal usage
		StackLogger (const StackLogger&) = delete; // no copies
		StackLogger& operator=(const StackLogger&) = delete; // no self-assignments
		StackLogger (StackLogger&&) = delete; // no move constructor
		StackLogger& operator=(StackLogger&&) = delete; // no move assignments

	public:
		StackLogger ();
		//~StackLogger ();

		void add (LogLevel logLevel, const std::string& event);

		void sendEvents (LogEventsReceiver& receiver, LogLevel logLevel);


		void delLogsOltherThan (int maxLogFileDays);
	};


	StackLogger stackLogger;


	StackLogger::StackLogger () : maxStoredEvents (stackSize)
	{
	}


	void StackLogger::sendEvents (LogEventsReceiver& receiver, LogLevel logLevel)
	{
		for (auto& event : events)
		{
			if (event.logLevel >= logLevel)
			{
				receiver.receive (event.date, event.event, event.logLevel);
			}
		}
	}
	void StackLogger::add (LogLevel logLevel, const  std::string& event)
	{
		EventContainer newEvent;

		auto now = std::chrono::system_clock::now ();
		//#if __cplusplus >= 202002L
#if true
		newEvent.date = format ("{}", now);
#else
		auto in_time_t = std::chrono::system_clock::to_time_t (now);
		struct tm buf;
		gmtime_s (&buf, &in_time_t);
		char str [100];
		strftime ((char*) str, sizeof (str), "%F %T UTC", &buf);

		newEvent.date = str;
#endif


		newEvent.event = event;
		newEvent.logLevel = logLevel;

		events.push_back (newEvent);

		if (events.size () > maxStoredEvents)
		{
			events.pop_front ();
		}
	}

	//-------------- LogMessageBuilder ----------------
	/**
	* Constructor
	*/
	LogMessageBuilder::LogMessageBuilder (BaseStreamLogger& logger) : logger (logger)
	{
		sendToLog = true;
	}

	/**
	* Move constructor
	*/
	LogMessageBuilder::LogMessageBuilder (LogMessageBuilder&& other) noexcept
		: logger (other.logger), message (std::move (other.message))
	{
		// Move constructor logic here
		sendToLog = true;
		other.sendToLog = false;
	}

	/**
	* Destructor, log the message
	*/
	LogMessageBuilder::~LogMessageBuilder ()
	{
		// If using string stream
		//logger.log (stream.str ());
		if (sendToLog) logger.log (message);
	}

	LogMessageBuilder& LogMessageBuilder::operator<<(const std::string& s)
	{
		this->message.append (s);
		return *this;
	}
	LogMessageBuilder& LogMessageBuilder::operator<<(int value)
	{
		this->message.append (std::to_string (value));
		return *this;
	}
	LogMessageBuilder& LogMessageBuilder::operator<<(double value)
	{
		this->message.append (std::to_string (value));
		return *this;
	}

	//-------------- BaseStreamLogger ----------------
	BaseStreamLogger::BaseStreamLogger (LogLevel level) : level (level)
	{
	}

	void BaseStreamLogger::log (const std::string& message)
	{
		//Send to console
		if (level >= consoleLevel)
		{
			std::cout << message << std::endl;
			//TODO: 
		}

		//Send to file
		if (level >= fileLevel)
		{
			//TODO:
		}

		//Send to stack
		if (level >= stackLevel)
		{
			stackLogger.add (level, message);
		}
	}


	LogMessageBuilder BaseStreamLogger::operator<<(const std::string& value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}
	LogMessageBuilder BaseStreamLogger::operator<<(int value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}
	LogMessageBuilder BaseStreamLogger::operator<<(double value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}

	//-------------- Event retransmission ----------------
	void retrieveLogEvents (LogEventsReceiver& receiver, BaseStreamLogger& logLevel)
	{
		stackLogger.sendEvents (receiver, logLevel.level);
	}
	void retrieveLogEvents (LogEventsReceiver& receiver, LogLevel logLevel)
	{
		stackLogger.sendEvents (receiver, logLevel);
	}





}