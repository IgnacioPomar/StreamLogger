/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "StreamLogger/StreamLoggerConsts.h"
#include "StackLogger.h"
#include "StreamLogger/StreamLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{

	//--------------  Static values: Configuration Vars ----------------

	bool gMultiThreadSafe    = DEFAULTS::MULTI_THREAD_SAFE;
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
