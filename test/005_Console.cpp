/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "TestUtils.h"

namespace
{
	// Captures std::clog
	class ClogCapture
	{
		public:
			ClogCapture()
			    : old (std::clog.rdbuf (buffer.rdbuf()))
			{
			}
			~ClogCapture()
			{
				std::clog.rdbuf (old);
			}
			std::string str () const
			{
				return buffer.str();
			}

		private:
			std::stringstream buffer;
			std::streambuf *old;
	};
}    // namespace

TEST_CASE ("Console output", "[console]")
{
	lggr::StackLogger logger;
	logger.setConsoleLevel (lggr::LL::WARN);

	SECTION ("Without colors")
	{
		logger.setColorMode (lggr::ColorMode::NEVER);
		ClogCapture capture;
		log (logger, lggr::LL::INFO, "Below the level");
		log (logger, lggr::LL::WARN, "Shown");

		std::string out = capture.str();
		CHECK (out.find ("Below the level") == std::string::npos);
		CHECK (out.find (" [WARN]\tShown\n") != std::string::npos);
		CHECK (out.find ('\033') == std::string::npos);
	}

#ifndef _WIN32
	SECTION ("With colors, everything goes to std::clog")
	{
		logger.setColorMode (lggr::ColorMode::ALWAYS);
		ClogCapture capture;
		log (logger, lggr::LL::WARN, "Yellow");

		std::string out = capture.str();
		CHECK (out.starts_with ("\033[33m"));
		CHECK (out.ends_with ("Yellow\n\033[0m"));
	}
#endif
}

TEST_CASE ("NO_COLOR disables the colors in AUTO mode", "[console]")
{
#ifdef _WIN32
	_putenv_s ("NO_COLOR", "1");
#else
	setenv ("NO_COLOR", "1", 1);
#endif
	lggr::StackLogger logger;
	logger.setColorMode (lggr::ColorMode::AUTO);
	CHECK_FALSE (logger.useColors);

	logger.setColorMode (lggr::ColorMode::ALWAYS);
	CHECK (logger.useColors);
}

TEST_CASE ("The console is flushed following its flush policy", "[console]")
{
	lggr::StackLogger logger;
	logger.setConsoleLevel (lggr::LL::TRACE);
	logger.setColorMode (lggr::ColorMode::NEVER);
	logger.setConsoleFlushInterval (std::chrono::milliseconds (0));
	ClogCapture capture;

	SECTION ("Default: trace and debug are buffered, info flushes")
	{
		log (logger, lggr::LL::DEBUG, "Buffered");
		CHECK (capture.str().empty());

		log (logger, lggr::LL::INFO, "Flushed");
		std::string out = capture.str();
		CHECK (out.find ("Buffered") != std::string::npos);
		CHECK (out.find ("Flushed") != std::string::npos);
		CHECK (out.find ("Buffered") < out.find ("Flushed"));
	}

	SECTION ("Every N events")
	{
		logger.setConsoleFlushEvery (lggr::LL::DEBUG, 2);
		log (logger, lggr::LL::DEBUG, "One");
		CHECK (capture.str().empty());
		log (logger, lggr::LL::DEBUG, "Two");
		CHECK (capture.str().find ("Two") != std::string::npos);
	}

	SECTION ("Manual flush")
	{
		log (logger, lggr::LL::TRACE, "Manual");
		CHECK (capture.str().empty());
		logger.flush();
		CHECK (capture.str().find ("Manual") != std::string::npos);
	}

	SECTION ("Independent of the file policy")
	{
		logger.setFlushEvery (lggr::LL::DEBUG, 1);    // File: does not affect the console
		log (logger, lggr::LL::DEBUG, "Still buffered");
		CHECK (capture.str().empty());
		logger.flushConsole();
	}

	SECTION ("A full buffer is written")
	{
		for (int i = 0; i < 1000; i++)
		{
			log (logger, lggr::LL::TRACE, "Filling the console buffer");
		}
		CHECK_FALSE (capture.str().empty());
		logger.flushConsole();
		CHECK (capture.str().size() > 1000 * std::string ("Filling the console buffer").size());
	}

#ifndef _WIN32
	SECTION ("The colors are buffered with the text")
	{
		logger.setColorMode (lggr::ColorMode::ALWAYS);
		log (logger, lggr::LL::DEBUG, "White");
		CHECK (capture.str().empty());
		log (logger, lggr::LL::WARN, "Yellow");

		std::string out = capture.str();
		CHECK (out.starts_with ("\033[37m"));
		CHECK (out.find ("White\n\033[0m\033[33m") != std::string::npos);
		CHECK (out.ends_with ("Yellow\n\033[0m"));
	}
#endif
}
