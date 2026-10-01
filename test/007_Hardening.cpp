/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <atomic>
#include <chrono>
#include <future>
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
			bool enabled   = true;
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

TEST_CASE ("An exception while logging does not escape the builder destructor", "[filter]")
{
	FakeLogger logger;
	logger.throwInLog = true;
	CHECK_NOTHROW (logger << "Lost");
}

TEST_CASE ("isEnabled follows the configuration", "[filter]")
{
	lggr::StackLogger logger;
	silence (logger);
	logger.setStackLevel (lggr::LL::WARN);
	CHECK_FALSE (logger.isEnabled (lggr::LL::INFO));
	CHECK (logger.isEnabled (lggr::LL::WARN));

	CollectingSubscriber collector;
	{
		auto subscription = logger.subscribe (collector, lggr::LL::DEBUG);
		CHECK (logger.isEnabled (lggr::LL::DEBUG));
		CHECK_FALSE (logger.isEnabled (lggr::LL::TRACE));
	}
	CHECK_FALSE (logger.isEnabled (lggr::LL::DEBUG));
}

TEST_CASE ("The static loggers know if they are enabled", "[filter]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);
	lggr::Config::setStackLevel (lggr::LL::INFO);
	CHECK_FALSE (lggr::trace.isEnabled());
	CHECK (lggr::info.isEnabled());

	CollectingSubscriber collector;
	auto subscription = lggr::subscribe (collector, lggr::LL::TRACE);
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
		auto subscription = lggr::subscribe (collector, lggr::LL::WARN);
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

//-------------- Subscribers ----------------

TEST_CASE ("A destroyed subscription receives nothing", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	CollectingSubscriber collector;
	{
		auto subscription = logger.subscribe (collector, lggr::LL::INFO);
		CHECK (subscription);
		log (logger, lggr::LL::INFO, "Received");
	}
	log (logger, lggr::LL::INFO, "Not received");
	CHECK (collector.texts() == std::vector<std::string> {"Received"});
	CHECK (logger.subscriberLevel == lggr::LL::OFF);
}

TEST_CASE ("A subscription can be moved and reset", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	CollectingSubscriber collector;

	lggr::Subscription outer;
	CHECK_FALSE (outer);
	{
		auto inner = logger.subscribe (collector, lggr::LL::INFO);
		outer      = std::move (inner);
	}
	log (logger, lggr::LL::INFO, "Received");
	outer.reset();
	CHECK_FALSE (outer);
	log (logger, lggr::LL::INFO, "Not received");
	CHECK (collector.texts() == std::vector<std::string> {"Received"});
}

TEST_CASE ("A subscription can outlive its logger", "[subscribers]")
{
	CollectingSubscriber collector;
	lggr::Subscription subscription;
	{
		lggr::StackLogger logger;
		silence (logger);
		subscription = logger.subscribe (collector, lggr::LL::INFO);
	}
	subscription.reset();    // Must not touch the destroyed logger
	CHECK_FALSE (subscription);
}

namespace
{
	// Blocks in the callback until released
	class BlockingSubscriber : public lggr::LogEventsSubscriber
	{
		public:
			std::promise<void> entered;
			std::shared_future<void> release;
			std::atomic<int> calls {0};
			std::atomic<bool> finished {false};

			void onLogEvent (const std::string &, const std::string, const lggr::LogLevel) override
			{
				if (calls++ == 0)
				{
					entered.set_value();
					release.wait();
				}
				finished = true;
			}
	};
}    // namespace

TEST_CASE ("Destroying a subscription waits for the running callback", "[subscribers][multithread]")
{
	lggr::StackLoggerMTSafe logger;
	silence (logger);

	std::promise<void> release;
	BlockingSubscriber subscriber;
	subscriber.release = release.get_future().share();

	auto subscription = std::make_unique<lggr::Subscription> (logger.subscribe (subscriber, lggr::LL::WARN));

	std::thread producer ([&logger] { log (logger, lggr::LL::WARN, "Blocking"); });
	subscriber.entered.get_future().wait();

	// The logger is not locked while the callback runs: other events are processed
	log (logger, lggr::LL::INFO, "Not for the subscriber");
	CHECK (pullTexts (logger, lggr::LL::TRACE).size() == 2);

	std::atomic<bool> unsubscribed {false};
	std::thread unsubscriber ([&] {
		subscription.reset();    // Destroys the Subscription
		unsubscribed = true;
	});

	std::this_thread::sleep_for (std::chrono::milliseconds (50));
	CHECK_FALSE (unsubscribed);    // Waiting for the callback

	release.set_value();
	unsubscriber.join();
	producer.join();
	CHECK (unsubscribed);
	CHECK (subscriber.finished);

	log (logger, lggr::LL::WARN, "After unsubscribing");
	CHECK (subscriber.calls == 1);
}

namespace
{
	// Logs inside the callback
	class EchoSubscriber : public lggr::LogEventsSubscriber
	{
		public:
			lggr::StackLogger *logger = nullptr;
			std::vector<std::string> received;

			void onLogEvent (const std::string &, const std::string logTxt, const lggr::LogLevel) override
			{
				received.push_back (logTxt);
				std::string echo = "Echo: " + logTxt;
				logger->log (lggr::LL::INFO, echo);
			}
	};
}    // namespace

TEST_CASE ("A subscriber can log without deadlock, and its events are not pushed", "[subscribers][multithread]")
{
	lggr::StackLoggerMTSafe logger;
	silence (logger);
	EchoSubscriber echo;
	echo.logger       = &logger;
	auto subscription = logger.subscribe (echo, lggr::LL::INFO);

	log (logger, lggr::LL::INFO, "Original");

	CHECK (echo.received == std::vector<std::string> {"Original"});
	CHECK (pullTexts (logger, lggr::LL::TRACE) == std::vector<std::string> {"Original", "Echo: Original"});
}

namespace
{
	class ThrowingSubscriber : public lggr::LogEventsSubscriber
	{
		public:
			void onLogEvent (const std::string &, const std::string, const lggr::LogLevel) override
			{
				throw std::runtime_error ("Expected test exception");
			}
	};

	// Unsubscribes itself in the first callback
	class SelfUnsubscriber : public lggr::LogEventsSubscriber
	{
		public:
			lggr::Subscription subscription;
			int calls = 0;

			void onLogEvent (const std::string &, const std::string, const lggr::LogLevel) override
			{
				++calls;
				subscription.reset();
			}
	};
}    // namespace

TEST_CASE ("An exception in a subscriber does not affect the others", "[subscribers]")
{
	lggr::StackLogger logger;
	silence (logger);
	ThrowingSubscriber thrower;
	CollectingSubscriber collector;
	auto subThrower   = logger.subscribe (thrower, lggr::LL::INFO);
	auto subCollector = logger.subscribe (collector, lggr::LL::INFO);

	CHECK_NOTHROW (log (logger, lggr::LL::INFO, "Event"));
	CHECK (collector.texts() == std::vector<std::string> {"Event"});
}

TEST_CASE ("A subscriber can unsubscribe inside its callback", "[subscribers]")
{
	lggr::StackLoggerMTSafe logger;
	silence (logger);
	SelfUnsubscriber subscriber;
	subscriber.subscription = logger.subscribe (subscriber, lggr::LL::INFO);

	log (logger, lggr::LL::INFO, "First");
	log (logger, lggr::LL::INFO, "Second");
	CHECK (subscriber.calls == 1);
	CHECK (logger.subscriberLevel == lggr::LL::OFF);
}

TEST_CASE ("The deprecated push interface still works", "[subscribers]")
{
	lggr::Config::setConsoleLevel (lggr::LL::OFF);
	CollectingSubscriber collector;

#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable : 4996)
#endif
	lggr::subscribePushEvents (collector, lggr::LL::FATAL);
	lggr::fatal << "Deprecated fatal";
	lggr::unsubscribePushEvents (collector);
#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#	pragma warning(pop)
#endif

	lggr::fatal << "Not received";
	CHECK (collector.texts() == std::vector<std::string> {"Deprecated fatal"});
}

//-------------- Date format ----------------

TEST_CASE ("The date has the same format with any compiler", "[date]")
{
	lggr::StackLogger logger;
	silence (logger);
	CollectingSubscriber collector;
	auto subscription = logger.subscribe (collector, lggr::LL::INFO);
	log (logger, lggr::LL::INFO, "Event");

	REQUIRE (collector.events.size() == 1);
	CHECK_THAT (collector.events [0].date, Matches (R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3} UTC)"));
}
