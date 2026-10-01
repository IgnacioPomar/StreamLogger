/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

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
