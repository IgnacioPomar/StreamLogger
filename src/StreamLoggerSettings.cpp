/*********************************************************************************************
 *	Name		: StackLoggerSettings.cpp
 *	Description	:
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "StreamLoggerConsts.h"
#include "StackLogger.h"
#include "StreamLogger.h"

namespace StreamLogger
{
	//--------------  Const values ----------------

	BaseStreamLogger trace (LogLevel::TRACE);
	BaseStreamLogger debug (LogLevel::DEBUG);
	BaseStreamLogger info (LogLevel::INFO);
	BaseStreamLogger warn (LogLevel::WARN);
	BaseStreamLogger error (LogLevel::ERROR);
	BaseStreamLogger fatal (LogLevel::FATAL);

	//--------------  Static values: Configuration Vars ----------------

	LogColor levelColors [6] = {DEFAULTS::COLOR_TRACE, DEFAULTS::COLOR_DEBUG, DEFAULTS::COLOR_INFO,
	                            DEFAULTS::COLOR_WARN,  DEFAULTS::COLOR_ERROR, DEFAULTS::COLOR_FATAL};

	StackLogger &getLogger ()
	{
		static StackLogger logger;
		return logger;

		/*
		// TODO: Make this in the future
		if (multiThreadSafe)
		{
		    static StackLoggerMTSafe logger;
		    return logger;
		}
		else
		{
		    static StackLogger logger;
		    return logger;
		}
		*/
	};

}    // namespace StreamLogger
