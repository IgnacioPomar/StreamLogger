/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <catch2/matchers/catch_matchers_string.hpp>

#include "TestUtils.h"

using Catch::Matchers::Matches;

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
