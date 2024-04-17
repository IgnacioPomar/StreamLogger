/*********************************************************************************************
 *	Name		: StreamLogger.h
 *	Description	: Stream logger Header
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <string>
#include <cstdint>

// If the lggrExportCfg.h is present, we are using it as a DLL os a static library
// If not, we are using the library as a embedded source
#if __has_include("lggrExportCfg.h")
#	include "lggrExportCfg.h"
#else
#	define LGGR_API
#	define LGGR_LOCAL
#endif

namespace StreamLogger
{
	//--------------  Enums ----------------
	enum class LogLevel : std::uint8_t
	{
		DontLog = 0xff,    // Highest value, used to indicate no logging should be performed.
		TRACE   = 0x00,    // For detailed information, useful during development.
		DEBUG   = 0x01,    // For diagnostic information, helpful in debugging.
		INFO    = 0x02,    // For general runtime information.
		WARN    = 0x03,    // For warnings, potential issues that are not errors.
		ERROR   = 0x04,    // For errors, issues that may affect operation but not critical.
		FATAL   = 0x05     // For critical issues, may require the program to terminate.
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
		// TODO: Add colors with background
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

	//--------------  Logger configuration ----------------
	class BaseStreamLogger;    // Forward declaration for this functions

	class LGGR_API Config
	{
		public:
			// If 0, there will be no stack at all
			static void setStackSize (int stackSize);

			static void setOutFile (const std::string fileName);    // It'll rotate by day if the template has a %d
			static void setOutPath (const std::string filePath);
			static void setLevelColor (LogLevel logLevel, LogColor logColor);

			static void setConsoleLevel (LogLevel logLevel);
			static void setFileLevel (LogLevel logLevel);
			static void setStackLevel (LogLevel logLevel);
	};

	//-------------- Classes to use externally ----------------

	class LogMessageBuilder
	{
		public:
			LGGR_API LogMessageBuilder (BaseStreamLogger &logger);
			LGGR_API LogMessageBuilder (const LogMessageBuilder &other) = delete;
			LGGR_API LogMessageBuilder (LogMessageBuilder &&other) noexcept;
			LGGR_API ~LogMessageBuilder();

			LGGR_API LogMessageBuilder &operator<< (const std::string &value);
			LGGR_API LogMessageBuilder &operator<< (int value);
			LGGR_API LogMessageBuilder &operator<< (double value);

		private:
			BaseStreamLogger &logger;

			// YAGNI: consider use a stringStream, but, worth to add additioonal dependencies?
			std::string message;
	};

	class LGGR_API BaseStreamLogger
	{
		public:
			BaseStreamLogger (LogLevel level);

			const LogLevel level;

			LogMessageBuilder operator<< (const std::string &value);
			LogMessageBuilder operator<< (int value);
			LogMessageBuilder operator<< (double value);

			void log (std::string &message);
	};

	extern LGGR_API BaseStreamLogger trace;
	extern LGGR_API BaseStreamLogger debug;
	extern LGGR_API BaseStreamLogger info;
	extern LGGR_API BaseStreamLogger warn;
	extern LGGR_API BaseStreamLogger error;
	extern LGGR_API BaseStreamLogger fatal;

	// YAGNI: conside move to another header file
	//  We cant use std::string_view because in multi-threading, the subyacent string could be deleted
	//--- Retrieve the generated events ---
	class LogEventsReceiver
	{
		public:
			virtual void receive (const std::string &date, const std::string logTxt, const LogLevel logLevel) = 0;
	};

	LGGR_API void retrieveLogEvents (LogEventsReceiver &receiver, const LogLevel logLevel);

	//--- Utility Functions ---
	const LGGR_API std::string &getLevelName (const LogLevel logLevel);

}    // namespace StreamLogger
