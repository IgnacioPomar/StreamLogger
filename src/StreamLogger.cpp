/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

// #include <filesystem>
// #include <fstream>
// #include <iostream>
// #include <list>
#include <string>

#include "StreamLoggerConsts.h"
#include "StreamLoggerSettings.h"
#include "StackLogger.h"
#include "StreamLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{

	//-------------- BaseStreamLogger ----------------
	BaseStreamLogger::BaseStreamLogger (LogLevel level)
	    : level (level)
	{
	}

	void BaseStreamLogger::log (std::string &message)
	{
		getLogger().log (level, message);
	}

	LogMessageBuilder BaseStreamLogger::operator<< (const std::string &value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}
	LogMessageBuilder BaseStreamLogger::operator<< (int value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}
	LogMessageBuilder BaseStreamLogger::operator<< (double value)
	{
		LogMessageBuilder tmpBuilder (*this);
		tmpBuilder << value;
		return std::move (tmpBuilder);
	}

}    // namespace IgnacioPomar::Util::StreamLogger
