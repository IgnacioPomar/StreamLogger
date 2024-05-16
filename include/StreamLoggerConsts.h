/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
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

#	include <cstdint>

#	ifdef ERROR
// Define a prefix to avoid conflict with the ERROR macro
#		define LOG_LEVEL_NEED_PREFIX
#	else
// No prefix needed
#
#	endif

namespace IgnacioPomar::Util::StreamLogger
{
	//--------------  Enums ----------------
	enum class LogLevel : std::uint8_t
	{
#	ifndef LOG_LEVEL_NEED_PREFIX
		OFF   = 0xff,    // Highest value, used to indicate no logging should be performed.
		TRACE = 0x00,    // For detailed information, useful during development.
		DEBUG = 0x01,    // For diagnostic information, helpful in debugging.
		INFO  = 0x02,    // For general runtime information.
		WARN  = 0x03,    // For warnings, potential issues that are not errors.
		ERROR = 0x04,    // For errors, issues that may affect operation but not critical.
		FATAL = 0x05     // For critical issues, may require the program to terminate.
#	else
		// Avoid Macro in other headers
		LL_OFF   = 0xff,
		LL_TRACE = 0x00,
		LL_DEBUG = 0x01,
		LL_INFO  = 0x02,
		LL_WARN  = 0x03,
		LL_ERROR = 0x04,
		LL_FATAL = 0x05
#	endif
	};

	// Alias for the enum
	using LL = LogLevel;

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
		constexpr const char *FILE_NAME {"%d_StreamedLog.log"};

		constexpr LogColor COLOR_DEBUG {LogColor::WHITE};
		constexpr LogColor COLOR_TRACE {LogColor::GREY};
		constexpr LogColor COLOR_INFO {LogColor::CYAN};
		constexpr LogColor COLOR_WARN {LogColor::YELLOW};
		constexpr LogColor COLOR_ERROR {LogColor::RED};
		constexpr LogColor COLOR_FATAL {LogColor::MAGENTA};

#	ifndef LOG_LEVEL_NEED_PREFIX
		constexpr LogLevel CONSOLE_LEVEL {LogLevel::INFO};
		constexpr LogLevel FILE_LEVEL {LogLevel::INFO};
		constexpr LogLevel STACK_LEVEL {LogLevel::INFO};
#	else
		constexpr LogLevel CONSOLE_LEVEL {LogLevel::LL_INFO};
		constexpr LogLevel FILE_LEVEL {LogLevel::LL_INFO};
		constexpr LogLevel STACK_LEVEL {LogLevel::LL_INFO};
#	endif

		constexpr int STACK_SIZE {1000};
	}    // namespace DEFAULTS

}    // namespace IgnacioPomar::Util::StreamLogger

#endif    // _LOGGER_CONSTANTS_H
