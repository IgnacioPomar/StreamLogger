/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef _STREAM_LOGGER_H_
#	define _STREAM_LOGGER_H_

#	include <chrono>
#	include <exception>
#	include <optional>
#	include <sstream>
#	include <string>
#	include <utility>
#	include "StreamLoggerConsts.h"
#	include "StreamLoggerInterfaces.h"

namespace IgnacioPomar::Util::StreamLogger
{

	//--------------  Logger configuration ----------------

	namespace Config
	{
		// Set this before any threads are started and do not change it afterwards.
		void setMultiThreadSafe (bool multiThreadSafe);

		// If 0, there will be no stack at all
		void setStackSize (unsigned int stackSize);

		void setOutFile (const std::string fileName);    // It'll rotate each day if the template has a %d
		void setOutPath (const std::string filePath);
		void setLevelColor (LogLevel logLevel, LogColor logColor);
		void setColorMode (ColorMode colorMode);

		void setConsoleLevel (LogLevel logLevel);
		void setFileLevel (LogLevel logLevel);
		void setStackLevel (LogLevel logLevel);

		// The log file is flushed after "events" events of that level (0: never by count)...
		void setFlushEvery (LogLevel logLevel, unsigned int events);
		// ... or when an event is written and the last flush is older than the interval (0: disabled)
		void setFlushInterval (std::chrono::milliseconds interval);

		// The same for the console (it has its own buffer)
		void setConsoleFlushEvery (LogLevel logLevel, unsigned int events);
		void setConsoleFlushInterval (std::chrono::milliseconds interval);

		// Flushes the console and the log file
		void flush ();
	};    // namespace Config

	namespace Internal
	{
		// Last resort error report (stderr): used where an exception can not be thrown, as destructors
		void reportError (const char *what) noexcept;
	}    // namespace Internal

	//-------------- Classes to use externally ----------------

	// forward declarations
	class LogMessageBuilder;
	class EventContainer;

	/**
	 * Interfaz to fill the logger message with stream
	 */
	class BaseStreamLogger
	{
		public:
			virtual void log (std::string &message) = 0;

			// False if the message would be discarded: use it to avoid expensive computations
			// (the arguments of << are always evaluated, but are not formatted if it is disabled)
			virtual bool isEnabled () const = 0;

			template <typename T> friend LogMessageBuilder operator<< (BaseStreamLogger &logger, const T &value);
	};

	/**
	 * A Event wich has been already stored in the stack. Allos to count its time
	 * Uppon destruction the event finishes
	 */
	class TimedEvent : public BaseStreamLogger
	{
		private:
			EventContainer *event;    // nullptr if the level was disabled: then, it does nothing
			bool started = false;

		public:
			~TimedEvent();
			explicit TimedEvent (EventContainer *event) noexcept;
			TimedEvent (const TimedEvent &)            = delete;    // the event is finished on destruction
			TimedEvent &operator= (const TimedEvent &) = delete;
			void log (std::string &message) override;
			bool isEnabled () const override;
	};

	/**
	 * Main interface to log messages: it'll be multiple instances of this class
	 */
	class StaticLogger : public BaseStreamLogger
	{
		public:
			StaticLogger (LogLevel level);

			const LogLevel level;

			void log (std::string &message) override;
			bool isEnabled () const override;

			TimedEvent startTimedEvent ();
	};

	//-------------- Instances of the loggers ----------------
	extern StaticLogger trace;
	extern StaticLogger debug;
	extern StaticLogger info;
	extern StaticLogger warn;
	extern StaticLogger error;
	extern StaticLogger fatal;

	//-------------- template functions ----------------

	class LogMessageBuilder
	{
		public:
			// If the logger is disabled, nothing is formatted (nor the stream is created)
			explicit LogMessageBuilder (BaseStreamLogger &logger)
			{
				if (logger.isEnabled())
				{
					this->logger = &logger;
					this->message.emplace();
				}
			};
			LogMessageBuilder (const LogMessageBuilder &other) = delete;
			LogMessageBuilder (LogMessageBuilder &&other) noexcept
			    : logger (std::exchange (other.logger, nullptr))
			    , message (std::move (other.message)) {};

			// The destructor can not throw: the errors are reported in stderr
			~LogMessageBuilder()
			{
				if (this->logger == nullptr)
				{
					return;
				}
				try
				{
					std::string msg = std::move (*this->message).str();
					if (!msg.empty())
					{
						logger->log (msg);
					}
				}
				catch (const std::exception &e)
				{
					Internal::reportError (e.what());
				}
				catch (...)
				{
					Internal::reportError ("unknown exception");
				}
			};

			// this funtion is a template: its outside the dll

			template <typename T> LogMessageBuilder &operator<< (const T &msg)
			{
				if (this->logger != nullptr)
				{
					*this->message << msg;
				}
				return *this;
			}

		private:
			BaseStreamLogger *logger = nullptr;

			std::optional<std::ostringstream> message;
	};

	template <typename T> LogMessageBuilder operator<< (BaseStreamLogger &logger, const T &value)
	{
		LogMessageBuilder tmpBuilder (logger);
		tmpBuilder << value;
		return tmpBuilder;
	}

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // _STREAM_LOGGER_H_
