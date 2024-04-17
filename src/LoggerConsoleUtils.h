/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "StreamLoggerConsts.h"

namespace IgnacioPomar::Util::StreamLogger
{
	// Console colors
	void setConsoleColor (LogColor color);
	void resetConsoleColor ();

	// const std::string &getLevelName (LogLevel logLevel)
}    // namespace IgnacioPomar::Util::StreamLogger
