/*********************************************************************************************
 *	Name		: StackLogger.h
 *	Description	: Stack Logger Header
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef STACKLOGGER_H
#	define STACKLOGGER_H

#	include <list>
#	include <string>
#	include <fstream>

#	include "StreamLoggerConsts.h"

namespace StreamLogger
{
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

			void setStackSize (unsigned int size);

		public:
			StackLogger();
			//~StackLogger ();
			// void delLogsOltherThan (int maxLogFileDays);

			void log (LogLevel logLevel, std::string &event);
			void sendEvents (LogEventsReceiver &receiver, LogLevel logLevel);
	};

}    // namespace StreamLogger
#endif    // STACKLOGGER_H
