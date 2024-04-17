/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef __STREAM_LOGGER_INTERFACES_H
#	define __STREAM_LOGGER_INTERFACES_H

#	if __has_include("lggrExportCfg.h")
#		include "lggrExportCfg.h"
#	else
#		define LGGR_API
#	endif

#	include <string>

#	include "StreamLoggerConsts.h"

namespace IgnacioPomar::Util::StreamLogger
{
	// ----------------------   Util functions -------------------------------------
	const LGGR_API std::string &getLevelName (const LogLevel logLevel);

	//  We cant use std::string_view because in multi-threading, the subyacent string could be deleted
	//--- Retrieve the generated events ---
	class LogEventsSubscriber
	{
		public:
			virtual void onLogEvent (const std::string &date, const std::string logTxt,
			                         const LogLevel logLevel) const = 0;
	};

	LGGR_API void pullLogEvents (LogEventsSubscriber &subscriber, const LogLevel logLevel);
	LGGR_API void subscribePushEvents (LogEventsSubscriber &subscriber, const LogLevel logLevel);

}    // namespace IgnacioPomar::Util::StreamLogger
#endif    // __STREAM_LOGGER_INTERFACES_H
