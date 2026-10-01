/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#include "StreamLogger/StreamLoggerConsts.h"

namespace IgnacioPomar::Util::StreamLogger
{
#ifdef _WIN32
	// The colors are console attributes, not part of the text: the colored lines can not be buffered
	constexpr bool IN_BAND_COLORS = false;
#else
	// The colors are ANSI sequences inside the text
	constexpr bool IN_BAND_COLORS = true;
#endif

	// ANSI sequences, to buffer colored text (empty if not IN_BAND_COLORS)
	const char *ansiColor (LogColor color);
	const char *ansiReset ();

	// Console colors (the console output is std::clog)
	void setConsoleColor (LogColor color);
	void resetConsoleColor ();

	// True if std::clog is a terminal and the NO_COLOR environment variable is not set (https://no-color.org)
	bool isColorConsole ();

	// const std::string &getLevelName (LogLevel logLevel)
}    // namespace IgnacioPomar::Util::StreamLogger
