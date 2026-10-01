/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "TestUtils.h"

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
