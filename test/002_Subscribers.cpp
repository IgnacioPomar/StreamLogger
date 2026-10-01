/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "TestUtils.h"

using trompeloeil::_;

TEST_CASE ("Push subscribers receive the events of their level or higher", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	MockSubscriber mock;
	logger.subscribePushEvents (mock, lggr::LL::WARN);

	REQUIRE_CALL (mock, onLogEvent (_, "warn", lggr::LL::WARN));
	REQUIRE_CALL (mock, onLogEvent (_, "fatal", lggr::LL::FATAL));

	log (logger, lggr::LL::INFO, "info");
	log (logger, lggr::LL::WARN, "warn");
	log (logger, lggr::LL::FATAL, "fatal");
}

TEST_CASE ("Each subscriber has its own level", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	MockSubscriber all;
	MockSubscriber errors;
	logger.subscribePushEvents (all, lggr::LL::TRACE);
	logger.subscribePushEvents (errors, lggr::LL::ERROR);

	REQUIRE_CALL (all, onLogEvent (_, "debug", lggr::LL::DEBUG));
	REQUIRE_CALL (all, onLogEvent (_, "error", lggr::LL::ERROR));
	REQUIRE_CALL (errors, onLogEvent (_, "error", lggr::LL::ERROR));

	log (logger, lggr::LL::DEBUG, "debug");
	log (logger, lggr::LL::ERROR, "error");
}

TEST_CASE ("A subscriber gets the events even if it is the only output with that level", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	logger.setStackLevel (lggr::LL::OFF);

	MockSubscriber mock;
	logger.subscribePushEvents (mock, lggr::LL::FATAL);

	REQUIRE_CALL (mock, onLogEvent (_, "fatal", lggr::LL::FATAL));

	log (logger, lggr::LL::INFO, "ignored");
	log (logger, lggr::LL::FATAL, "fatal");
}

TEST_CASE ("Unsubscribed subscribers receive nothing", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	logger.setStackLevel (lggr::LL::OFF);

	MockSubscriber info;
	MockSubscriber errors;
	logger.subscribePushEvents (info, lggr::LL::INFO);
	logger.subscribePushEvents (errors, lggr::LL::ERROR);

	logger.unsubscribePushEvents (info);
	CHECK (logger.subscriberLevel == lggr::LL::ERROR);

	REQUIRE_CALL (errors, onLogEvent (_, "error", lggr::LL::ERROR));
	log (logger, lggr::LL::INFO, "info");
	log (logger, lggr::LL::ERROR, "error");

	logger.unsubscribePushEvents (errors);
	CHECK (logger.subscriberLevel == lggr::LL::OFF);
	CHECK (logger.effectiveLevel == lggr::LL::OFF);
	log (logger, lggr::LL::FATAL, "fatal");
}

TEST_CASE ("Pull subscribers receive the stored events in order", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	log (logger, lggr::LL::INFO, "first");
	log (logger, lggr::LL::ERROR, "second");

	MockSubscriber mock;
	trompeloeil::sequence seq;
	REQUIRE_CALL (mock, onLogEvent (_, "first", lggr::LL::INFO)).IN_SEQUENCE (seq);
	REQUIRE_CALL (mock, onLogEvent (_, "second", lggr::LL::ERROR)).IN_SEQUENCE (seq);

	logger.sendEvents (mock, lggr::LL::TRACE);
}

TEST_CASE ("Pushed and pulled events have the same date", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	CollectingSubscriber pushed;
	logger.subscribePushEvents (pushed, lggr::LL::INFO);

	log (logger, lggr::LL::INFO, "event");

	CollectingSubscriber pulled;
	logger.sendEvents (pulled, lggr::LL::INFO);

	REQUIRE (pushed.events.size() == 1);
	REQUIRE (pulled.events.size() == 1);
	CHECK (pushed.events [0].date == pulled.events [0].date);
}

TEST_CASE ("Subscribers through the public interface", "[subscribers]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);

	MockSubscriber mock;
	{
		ScopedPushSubscription subscription (mock, lggr::LL::FATAL);
		REQUIRE_CALL (mock, onLogEvent (_, "Public fatal", lggr::LL::FATAL));
		lggr::error << "Public error";
		lggr::fatal << "Public fatal";
	}
	lggr::fatal << "Not received: unsubscribed";

	CollectingSubscriber collector;
	lggr::pullLogEvents (collector, lggr::LL::ERROR);
	auto texts = collector.texts();
	CHECK (std::find (texts.begin(), texts.end(), "Public error") != texts.end());
	CHECK (std::find (texts.begin(), texts.end(), "Public fatal") != texts.end());
}
