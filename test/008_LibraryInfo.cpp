/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "TestUtils.h"

#include <cctype>
#include <format>

TEST_CASE ("The version macros are consistent", "[version]")
{
	CHECK (std::string (STREAMLOGGER_NAME) == "StreamLogger");
	CHECK (std::string (STREAMLOGGER_DESCRIPTION) != "");
	CHECK (std::string (STREAMLOGGER_VERSION)
	       == std::format ("{}.{}.{}", STREAMLOGGER_VERSION_MAJOR, STREAMLOGGER_VERSION_MINOR,
	                       STREAMLOGGER_VERSION_PATCH));
}

TEST_CASE ("The loaded library reports the version it was compiled with", "[version]")
{
	const lggr::LibraryInfo &info = lggr::getLibraryInfo();

	// The tester is built with the library: both versions must be the same
	CHECK (std::string (info.name) == STREAMLOGGER_NAME);
	CHECK (std::string (info.description) == STREAMLOGGER_DESCRIPTION);
	CHECK (std::string (info.version) == STREAMLOGGER_VERSION);
	CHECK (info.versionMajor == STREAMLOGGER_VERSION_MAJOR);
	CHECK (info.versionMinor == STREAMLOGGER_VERSION_MINOR);
	CHECK (info.versionPatch == STREAMLOGGER_VERSION_PATCH);
}

TEST_CASE ("The build date is YYYY-MM-DD HH:MM:SS", "[version]")
{
	const std::string buildDate = lggr::getLibraryInfo().buildDate;

	REQUIRE (buildDate.size() == 19);
	for (std::size_t i : {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18})
	{
		CHECK (std::isdigit (static_cast<unsigned char> (buildDate [i])));
	}
	CHECK (buildDate.substr (4, 1) == "-");
	CHECK (buildDate.substr (7, 1) == "-");
	CHECK (buildDate.substr (10, 1) == " ");
	CHECK (buildDate.substr (13, 1) == ":");
	CHECK (buildDate.substr (16, 1) == ":");
	CHECK (buildDate.substr (5, 2) >= "01");
	CHECK (buildDate.substr (5, 2) <= "12");
}
