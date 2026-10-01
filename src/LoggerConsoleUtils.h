/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#include "StreamLogger/StreamLoggerConsts.h"

namespace IgnacioPomar::Util::StreamLogger
{
	// Console colors (the console output is std::clog)
	void setConsoleColor (LogColor color);
	void resetConsoleColor ();

	// True if std::clog is a terminal and the NO_COLOR environment variable is not set (https://no-color.org)
	bool isColorConsole ();

	// const std::string &getLevelName (LogLevel logLevel)
}    // namespace IgnacioPomar::Util::StreamLogger
