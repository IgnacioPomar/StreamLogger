/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "StreamLoggerConsts.h"
#include "StackLogger.h"
#include "StreamLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{
	//--------------  Const values ----------------

	StaticLogger trace (LogLevel::TRACE);
	StaticLogger debug (LogLevel::DEBUG);
	StaticLogger info (LogLevel::INFO);
	StaticLogger warn (LogLevel::WARN);
	StaticLogger error (LogLevel::ERROR);
	StaticLogger fatal (LogLevel::FATAL);

	//--------------  Static values: Configuration Vars ----------------

	bool gMultiThreadSafe    = false;
	bool isLoggerInitialized = false;

	StackLogger &initSTDLogger ()
	{
		static StackLogger logger;
		isLoggerInitialized = true;
		return logger;
	};

	StackLogger &initMTSafeLogger ()
	{
		static StackLoggerMTSafe loggerMTSafe;
		isLoggerInitialized = true;
		return loggerMTSafe;
	};

	StackLogger &getLogger ()
	{
		static StackLogger &logger = (gMultiThreadSafe) ? initMTSafeLogger() : initSTDLogger();
		return logger;
	}

}    // namespace IgnacioPomar::Util::StreamLogger
