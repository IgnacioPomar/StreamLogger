/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <chrono>

#include <catch2/matchers/catch_matchers_string.hpp>

#include "TestUtils.h"

using trompeloeil::_;
using Catch::Matchers::Matches;

TEST_CASE ("A timed event is sent when it starts and when it finishes", "[timed]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);

	MockSubscriber mock;
	auto subscription = lggr::subscribe (mock, lggr::LL::INFO);

	trompeloeil::sequence seq;
	REQUIRE_CALL (mock, onLogEvent (_, "Task 1", lggr::LL::INFO)).IN_SEQUENCE (seq);
	REQUIRE_CALL (mock, onLogEvent (_, "Inside the task", lggr::LL::INFO)).IN_SEQUENCE (seq);
	REQUIRE_CALL (mock, onLogEvent (_, trompeloeil::re ("^Task 1, more info\tDone in: "), lggr::LL::INFO))
	    .IN_SEQUENCE (seq);

	{
		auto timedEvt = lggr::info.startTimedEvent();
		timedEvt << "Task " << 1;
		timedEvt << ", more info";    // Second line: only added to the event
		lggr::info << "Inside the task";
	}
}

TEST_CASE ("A timed event below the subscriber level is not sent", "[timed]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);

	MockSubscriber mock;
	auto subscription = lggr::subscribe (mock, lggr::LL::WARN);

	auto timedEvt = lggr::debug.startTimedEvent();
	timedEvt << "Not sent";
}

TEST_CASE ("A running timed event is not removed from the stack", "[timed]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);
	lggr::Config::setStackLevel (lggr::LL::INFO);
	lggr::Config::setStackSize (2);

	auto contains = [] (const std::string &txt) {
		CollectingSubscriber collector;
		lggr::pullLogEvents (collector, lggr::LL::TRACE);
		auto texts = collector.texts();
		return std::find (texts.begin(), texts.end(), txt) != texts.end();
	};

	{
		auto timedEvt = lggr::info.startTimedEvent();
		timedEvt << "Long task";

		for (int i = 0; i < 5; i++)
		{
			lggr::info << "Filler " << i;
		}

		CHECK (contains ("Long task"));
		CHECK_FALSE (contains ("Filler 0"));
		CHECK (contains ("Filler 4"));
	}

	// Once finished, it is a normal event
	lggr::info << "After";
	lggr::info << "the task";
	CHECK_FALSE (contains ("Long task"));

	lggr::Config::setStackSize (lggr::DEFAULTS::STACK_SIZE);
}

TEST_CASE ("Elapsed time format", "[timed]")
{
	using namespace std::chrono_literals;

	lggr::StackLogger logger;
	lggr::EventContainer event (lggr::LL::INFO);

	SECTION ("Hours, minutes, seconds and milliseconds")
	{
		event.timePoint = std::chrono::system_clock::now() - (1h + 2min + 3s + 400ms);
		logger.fillElapsedTime (event);
		CHECK_THAT (event.usedTimeTxt, Matches (R"(1h 2' 3" 4\d\dms)"));
	}

	SECTION ("Only milliseconds")
	{
		event.timePoint = std::chrono::system_clock::now() - 250ms;
		logger.fillElapsedTime (event);
		CHECK_THAT (event.usedTimeTxt, Matches (R"(2\d\dms)"));
	}

	SECTION ("Less than a millisecond")
	{
		event.timePoint = std::chrono::system_clock::now();
		logger.fillElapsedTime (event);
		CHECK (event.usedTimeTxt == "0ms");
	}

	CHECK (event.eventType == lggr::EVENT_TYPE_TIMED_FINISHED);
}
