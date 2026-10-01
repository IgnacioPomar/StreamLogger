/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <stdexcept>

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
