/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <atomic>
#include <stdexcept>
#include <thread>

#include <catch2/matchers/catch_matchers_string.hpp>

#include "TestUtils.h"

using Catch::Matchers::Matches;

namespace
{
	// Counts how many times it is formatted
	struct Counted
	{
			int *count;
	};
	std::ostream &operator<< (std::ostream &os, const Counted &counted)
	{
		++*counted.count;
		return os << "counted";
	}

	class FakeLogger : public lggr::BaseStreamLogger
	{
		public:
			bool enabled    = true;
			bool throwInLog = false;
			std::vector<std::string> logged;

			void log (std::string &message) override
			{
				if (throwInLog)
				{
					throw std::runtime_error ("Expected test exception");
				}
				logged.push_back (message);
			}
			bool isEnabled () const override
			{
				return enabled;
			}
	};

	std::vector<std::string> pullAll ()
	{
		CollectingSubscriber collector;
		lggr::pullLogEvents (collector, lggr::LL::TRACE);
		return collector.texts();
	}

	bool contains (const std::vector<std::string> &texts, const std::string &txt)
	{
		return std::find (texts.begin(), texts.end(), txt) != texts.end();
	}
}    // namespace

//-------------- Filter before format ----------------

TEST_CASE ("A disabled logger does not format the message", "[filter]")
{
	FakeLogger logger;
	int count = 0;

	logger.enabled = false;
	logger << "Value: " << Counted {&count};
	CHECK (count == 0);
	CHECK (logger.logged.empty());

	logger.enabled = true;
	logger << "Value: " << Counted {&count};
	CHECK (count == 1);
	CHECK (logger.logged == std::vector<std::string> {"Value: counted"});
}

TEST_CASE ("An empty message is not logged", "[filter]")
{
	FakeLogger logger;
	logger << "";
	CHECK (logger.logged.empty());
}

TEST_CASE ("isEnabled follows the configuration", "[filter]")
{
	lggr::StackLogger logger;
	silence (logger);
	logger.setStackLevel (lggr::LL::WARN);
	CHECK_FALSE (logger.isEnabled (lggr::LL::INFO));
	CHECK (logger.isEnabled (lggr::LL::WARN));

	CollectingSubscriber collector;
	logger.subscribePushEvents (collector, lggr::LL::DEBUG);
	CHECK (logger.isEnabled (lggr::LL::DEBUG));
	CHECK_FALSE (logger.isEnabled (lggr::LL::TRACE));
	logger.unsubscribePushEvents (collector);
	CHECK_FALSE (logger.isEnabled (lggr::LL::DEBUG));
}

TEST_CASE ("The static loggers know if they are enabled", "[filter]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);
	lggr::Config::setStackLevel (lggr::LL::INFO);
	CHECK_FALSE (lggr::trace.isEnabled());
	CHECK (lggr::info.isEnabled());

	CollectingSubscriber collector;
	ScopedPushSubscription subscription (collector, lggr::LL::TRACE);
	CHECK (lggr::trace.isEnabled());
}

//-------------- Timed events ----------------

TEST_CASE ("A disabled timed event does nothing", "[timed]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);
	lggr::Config::setStackLevel (lggr::LL::INFO);

	{
		auto timedEvt = lggr::trace.startTimedEvent();
		CHECK_FALSE (timedEvt.isEnabled());
		timedEvt << "Disabled timed event";
	}
	CHECK_FALSE (contains (pullAll(), "Disabled timed event"));
}

TEST_CASE ("A timed event never started is discarded", "[timed]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);
	lggr::Config::setStackLevel (lggr::LL::INFO);

	auto before = pullAll().size();
	{
		auto timedEvt = lggr::info.startTimedEvent();
		CHECK (timedEvt.isEnabled());
		CHECK (pullAll().size() == before);    // Not started: not visible
	}
	lggr::info << "After the discarded event";
	auto texts = pullAll();
	CHECK_FALSE (contains (texts, ""));
	CHECK (contains (texts, "After the discarded event"));
}

TEST_CASE ("Timed events survive a small stack with concurrent logs", "[timed][multithread]")
{
	// It used to be a use-after-free: the event was removed before being marked as running
	lggr::Config::setConsoleLevel (lggr::LL::OFF);
	lggr::Config::setStackLevel (lggr::LL::INFO);
	lggr::Config::setStackSize (1);

	constexpr int ITERATIONS = 2000;
	std::atomic<bool> stop {false};

	std::vector<std::thread> loggers;
	for (int t = 0; t < 3; t++)
	{
		loggers.emplace_back ([&stop] {
			while (!stop)
			{
				lggr::info << "Filler";
			}
		});
	}

	CollectingSubscriber collector;
	{
		ScopedPushSubscription subscription (collector, lggr::LL::WARN);
		for (int i = 0; i < ITERATIONS; i++)
		{
			auto timedEvt = lggr::warn.startTimedEvent();
			timedEvt << "Task " << i;
			timedEvt << " more";
		}
	}
	stop = true;
	for (auto &thread : loggers)
	{
		thread.join();
	}

	// Start and finish of each event
	REQUIRE (collector.events.size() == 2 * ITERATIONS);
	CHECK (collector.events [1].txt.starts_with ("Task 0 more\tDone in: "));

	lggr::Config::setStackSize (lggr::DEFAULTS::STACK_SIZE);
}

//-------------- Errors ----------------

TEST_CASE ("An exception while logging does not escape the builder destructor", "[errors]")
{
	FakeLogger logger;
	logger.throwInLog = true;
	CHECK_NOTHROW (logger << "Lost");
}

//-------------- Date format ----------------

TEST_CASE ("The date has the same format with any compiler", "[date]")
{
	lggr::StackLogger logger;
	silence (logger);
	log (logger, lggr::LL::INFO, "Event");

	CollectingSubscriber collector;
	logger.sendEvents (collector, lggr::LL::INFO);
	REQUIRE (collector.events.size() == 1);
	CHECK_THAT (collector.events [0].date, Matches (R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3} UTC)"));
}
