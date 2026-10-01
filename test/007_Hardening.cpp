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
	class FakeLogger : public lggr::BaseStreamLogger
	{
		public:
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
	};
}    // namespace

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
