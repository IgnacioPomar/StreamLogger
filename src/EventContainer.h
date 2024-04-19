/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef _EVENT_CONTAINER_H
#	define _EVENT_CONTAINER_H

#	include <string>
#	include <chrono>
#	include <cstdint>

#	include "StreamLoggerConsts.h"

namespace IgnacioPomar::Util::StreamLogger
{
	typedef std::chrono::system_clock::time_point TimePoint;

	enum class EventType : std::uint8_t
	{
		NORMAL         = 0,
		TIMED_RUNNING  = 0b1000'0000,
		TIMED_FINISHED = 0b1100'0000

	};

	constexpr std::uint8_t EVENT_TYPE_NORMAL  = 0;
	constexpr std::uint8_t EVENT_TYPE_RUNNING = 0b0100'0000;
	constexpr std::uint8_t EVENT_TYPE_TIMED   = 0b1000'0000;

	constexpr std::uint8_t EVENT_TYPE_TIMED_RUNNING  = EVENT_TYPE_TIMED & EVENT_TYPE_RUNNING;
	constexpr std::uint8_t EVENT_TYPE_TIMED_FINISHED = EVENT_TYPE_TIMED;

	/**
	 * Contain the info for a single event
	 */
	class EventContainer
	{
		public:
			EventContainer (const LogLevel logLevel)
			    : logLevel (logLevel)
			{
			}

			// Storing the date as a string to avoid reformating each time we send it
			// And also as timePoint to detect rotation in filename
			TimePoint timePoint;
			std::string date;
			std::string event;
			LogLevel logLevel;

			std::uint8_t eventType = EVENT_TYPE_NORMAL;

			//--- Timed event properties ---
			TimePoint endTimePoint;
			std::string usedTimeTxt;
	};
}    // namespace IgnacioPomar::Util::StreamLogger
#endif
