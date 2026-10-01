/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>

#include "StreamLogger/StreamLogger.h"
#include "StackLogger.h"

namespace lggr = IgnacioPomar::Util::StreamLogger;

class MockSubscriber : public lggr::LogEventsSubscriber
{
	public:
		MAKE_MOCK3 (onLogEvent, void (const std::string &, const std::string, const lggr::LogLevel), override);
};

// Fake: stores the received events
class CollectingSubscriber : public lggr::LogEventsSubscriber
{
	public:
		struct Event
		{
				std::string date;
				std::string txt;
				lggr::LogLevel level;
		};
		std::vector<Event> events;

		void onLogEvent (const std::string &date, const std::string logTxt, const lggr::LogLevel logLevel) override
		{
			events.push_back ({date, logTxt, logLevel});
		}

		std::vector<std::string> texts () const
		{
			std::vector<std::string> result;
			for (auto &event : events)
			{
				result.push_back (event.txt);
			}
			return result;
		}
};

// Isolated logger, with no output besides the stack
inline void silence (lggr::StackLogger &logger)
{
	logger.setConsoleLevel (lggr::LL::OFF);
	logger.setFileLevel (lggr::LL::OFF);
}

inline void log (lggr::StackLogger &logger, lggr::LogLevel logLevel, std::string msg)
{
	logger.log (logLevel, msg);
}

inline std::vector<std::string> pullTexts (lggr::StackLogger &logger, lggr::LogLevel logLevel)
{
	CollectingSubscriber collector;
	logger.sendEvents (collector, logLevel);
	return collector.texts();
}
