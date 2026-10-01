/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <thread>

#include "TestUtils.h"

TEST_CASE ("The MT safe logger does not lose events", "[multithread]")
{
	constexpr int THREADS = 4;
	constexpr int EVENTS  = 1000;

	lggr::StackLoggerMTSafe logger;
	silence (logger);
	logger.setStackSize (THREADS * EVENTS);

	CollectingSubscriber pushed;
	logger.subscribePushEvents (pushed, lggr::LL::INFO);

	std::vector<std::thread> threads;
	for (int t = 0; t < THREADS; t++)
	{
		threads.emplace_back ([&logger, t] {
			for (int i = 0; i < EVENTS; i++)
			{
				log (logger, lggr::LL::INFO, std::to_string (t) + ":" + std::to_string (i));
			}
		});
	}
	for (auto &thread : threads)
	{
		thread.join();
	}

	CHECK (pushed.events.size() == THREADS * EVENTS);
	CHECK (pullTexts (logger, lggr::LL::TRACE).size() == THREADS * EVENTS);
}

TEST_CASE ("Multi-thread safe is the default", "[multithread]")
{
	CHECK (lggr::DEFAULTS::MULTI_THREAD_SAFE);
	CHECK (dynamic_cast<lggr::StackLoggerMTSafe *> (&lggr::getLogger()) != nullptr);
}
