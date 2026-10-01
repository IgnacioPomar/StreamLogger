/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "TestUtils.h"

using Texts = std::vector<std::string>;

TEST_CASE ("The stack only keeps the events of its level or higher", "[levels]")
{
	lggr::StackLogger logger;
	silence (logger);
	logger.setStackLevel (lggr::LL::WARN);

	log (logger, lggr::LL::INFO, "info");
	log (logger, lggr::LL::WARN, "warn");
	log (logger, lggr::LL::FATAL, "fatal");

	CHECK (pullTexts (logger, lggr::LL::TRACE) == Texts {"warn", "fatal"});
}

TEST_CASE ("Pulling filters by level", "[levels]")
{
	lggr::StackLogger logger;
	silence (logger);
	logger.setStackLevel (lggr::LL::TRACE);

	log (logger, lggr::LL::DEBUG, "debug");
	log (logger, lggr::LL::INFO, "info");
	log (logger, lggr::LL::ERROR, "error");

	CHECK (pullTexts (logger, lggr::LL::INFO) == Texts {"info", "error"});
	CHECK (pullTexts (logger, lggr::LL::OFF).empty());
}

TEST_CASE ("The stack keeps only the last events", "[levels]")
{
	lggr::StackLogger logger;
	silence (logger);
	logger.setStackSize (3);

	for (int i = 1; i <= 5; i++)
	{
		log (logger, lggr::LL::INFO, std::to_string (i));
	}
	CHECK (pullTexts (logger, lggr::LL::TRACE) == Texts {"3", "4", "5"});

	SECTION ("Reducing the stack size removes the oldest events")
	{
		logger.setStackSize (1);
		CHECK (pullTexts (logger, lggr::LL::TRACE) == Texts {"5"});
	}

	SECTION ("A stack of size 0 keeps nothing")
	{
		logger.setStackSize (0);
		log (logger, lggr::LL::FATAL, "fatal");
		CHECK (pullTexts (logger, lggr::LL::TRACE).empty());
		CHECK (logger.stackLevel == lggr::LL::OFF);
	}
}

TEST_CASE ("The effective level is the lowest of all the outputs", "[levels]")
{
	lggr::StackLogger logger;
	logger.setStackLevel (lggr::LL::ERROR);
	logger.setConsoleLevel (lggr::LL::WARN);
	logger.setFileLevel (lggr::LL::OFF);
	CHECK (logger.effectiveLevel == lggr::LL::WARN);

	logger.addSubscriberLevel (lggr::LL::DEBUG);
	CHECK (logger.effectiveLevel == lggr::LL::DEBUG);

	logger.resetSubscriberLevel();
	CHECK (logger.effectiveLevel == lggr::LL::WARN);

	logger.setConsoleLevel (lggr::LL::OFF);
	logger.setStackLevel (lggr::LL::OFF);
	CHECK (logger.effectiveLevel == lggr::LL::OFF);
}

TEST_CASE ("Level names", "[levels]")
{
	CHECK (lggr::getLevelName (lggr::LL::TRACE) == "TRACE");
	CHECK (lggr::getLevelName (lggr::LL::DEBUG) == "DEBUG");
	CHECK (lggr::getLevelName (lggr::LL::INFO) == "INFO");
	CHECK (lggr::getLevelName (lggr::LL::WARN) == "WARN");
	CHECK (lggr::getLevelName (lggr::LL::ERROR) == "ERROR");
	CHECK (lggr::getLevelName (lggr::LL::FATAL) == "FATAL");
	CHECK (lggr::getLevelName (lggr::LL::OFF) == "OFF");
}

TEST_CASE ("OFF has no color", "[levels]")
{
	lggr::StackLogger logger;
	logger.setLevelColor (lggr::LL::OFF, lggr::LogColor::GREEN);    // Must be ignored (it was an overflow)
	logger.setLevelColor (lggr::LL::INFO, lggr::LogColor::GREEN);
	CHECK (logger.levelColors [static_cast<int> (lggr::LL::INFO)] == lggr::LogColor::GREEN);
}

TEST_CASE ("The stream interface builds the message", "[levels]")
{
	CollectingSubscriber collector;
	ScopedPushSubscription subscription (collector, lggr::LL::WARN);
	lggr::Config::setConsoleLevel (lggr::LL::OFF);

	lggr::warn << "Value: " << 42 << ", " << 1.5;
	lggr::info << "Ignored by the subscriber";

	REQUIRE (collector.events.size() == 1);
	CHECK (collector.events [0].txt == "Value: 42, 1.5");
	CHECK (collector.events [0].level == lggr::LL::WARN);
	CHECK_FALSE (collector.events [0].date.empty());
}
