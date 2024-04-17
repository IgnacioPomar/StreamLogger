/*********************************************************************************************
 *	Name		: LoggerConsts.h
 *	Description	: Logger Constants
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#pragma once
#ifndef _LOGGER_CONSTANTS_H
#	define _LOGGER_CONSTANTS_H

#	if __has_include("lggrExportCfg.h")
#		include "lggrExportCfg.h"
#	else
#		define LGGR_API
#	endif

#	include <string>
#	include <cstdint>

namespace StreamLogger
{
	//--------------  Enums ----------------
	enum class LogLevel : std::uint8_t
	{
		OFF   = 0xff,    // Highest value, used to indicate no logging should be performed.
		TRACE = 0x00,    // For detailed information, useful during development.
		DEBUG = 0x01,    // For diagnostic information, helpful in debugging.
		INFO  = 0x02,    // For general runtime information.
		WARN  = 0x03,    // For warnings, potential issues that are not errors.
		ERROR = 0x04,    // For errors, issues that may affect operation but not critical.
		FATAL = 0x05     // For critical issues, may require the program to terminate.
	};

	// Alias for the enum
	using LL = LogLevel;

	// Util functions
	const LGGR_API std::string &getLevelName (const LogLevel logLevel);

	enum class LogColor : std::uint8_t
	{
		BLACK,    // Not to use, it wold display invisible text
		WHITE,
		GREY,
		// DARKGREY,
		RED,
		LIGHTRED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		// MAYBE: Add colors with background
	};

	//--------------  Default Values ----------------
	namespace DEFAULTS
	{
		inline constexpr const char *FILE_NAME = "%d_StreamedLog.log";

		inline constexpr LogColor COLOR_DEBUG = LogColor::WHITE;
		inline constexpr LogColor COLOR_TRACE = LogColor::GREY;
		inline constexpr LogColor COLOR_INFO  = LogColor::CYAN;
		inline constexpr LogColor COLOR_WARN  = LogColor::YELLOW;
		inline constexpr LogColor COLOR_ERROR = LogColor::RED;
		inline constexpr LogColor COLOR_FATAL = LogColor::MAGENTA;

		inline constexpr LogLevel CONSOLE_LEVEL = LogLevel::INFO;
		inline constexpr LogLevel FILE_LEVEL    = LogLevel::INFO;
		inline constexpr LogLevel STACK_LEVEL   = LogLevel::INFO;

		inline constexpr int STACK_SIZE = 1000;
	}    // namespace DEFAULTS

	// YAGNI: conside move to another header file
	//  We cant use std::string_view because in multi-threading, the subyacent string could be deleted
	//--- Retrieve the generated events ---
	class LogEventsReceiver
	{
		public:
			virtual void receive (const std::string &date, const std::string logTxt, const LogLevel logLevel) = 0;
	};

	LGGR_API void retrieveLogEvents (LogEventsReceiver &receiver, const LogLevel logLevel);

}    // namespace StreamLogger

#endif    // _LOGGER_CONSTANTS_H
