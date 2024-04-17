/*********************************************************************************************
 *	Name		: StreamLogger.h
 *	Description	: Stream logger Implementation
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

namespace StreamLogger
{
	//--------------  Configuration functions ----------------
	void Config::setLevelColor (LogLevel logLevel, LogColor logColor)
	{
		levelColors [static_cast<int> (logLevel)] = logColor;
	}

	void Config::setConsoleLevel (LogLevel logLevel)
	{
		consoleLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}
	void Config::setFileLevel (LogLevel logLevel)
	{
		fileLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}
	void Config::setStackLevel (LogLevel logLevel)
	{
		stackLevel = (logLevel > LogLevel::FATAL) ? LogLevel::FATAL : logLevel;
	}

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

}    // namespace StreamLogger
