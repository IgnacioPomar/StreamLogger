#include <string>

namespace StreamLogger
{
	//--------------  Enums ----------------
	enum class LogLevel : unsigned char
	{
		DontLog = 0xff, // Highest value, used to indicate no logging should be performed.
		TRACE = 0x00,   // For detailed information, useful during development.
		DEBUG = 0x01,   // For diagnostic information, helpful in debugging.
		INFO = 0x02,    // For general runtime information.
		WARN = 0x03,    // For warnings, potential issues that are not errors.
		ERROR = 0x04,   // For errors, issues that may affect operation but not critical.
		FATAL = 0x05    // For critical issues, may require the program to terminate.
	};

	enum class LogColor : unsigned char
	{
		BLACK,		//Not to use, it wold display invisible text
		WHITE,
		GREY,
		LIGHTGREY,
		RED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		//TODO: Add colors with background
	};

	//--------------  Default Values ----------------
	namespace DEFAULTS
	{
		inline constexpr const char* FILE_NAME = "%d_StreamedLog.log";

		inline constexpr LogColor COLOR_DEBUG = LogColor::GREY;
		inline constexpr LogColor COLOR_TRACE = LogColor::LIGHTGREY;
		inline constexpr LogColor COLOR_INFO = LogColor::WHITE;
		inline constexpr LogColor COLOR_WARN = LogColor::YELLOW;
		inline constexpr LogColor COLOR_ERROR = LogColor::RED;
		inline constexpr LogColor COLOR_FATAL = LogColor::RED;

		inline constexpr LogLevel CONSOLE_LEVEL = LogLevel::INFO;
		inline constexpr LogLevel FILE_LEVEL = LogLevel::INFO;
		inline constexpr LogLevel STACK_LEVEL = LogLevel::INFO;

		inline constexpr int STACK_SIZE = 1000;
	}


	//--------------  Logger configuration ----------------
	class BaseStreamLogger; //Forward declaration for this functions

	class Config
	{
	public:
		//If 0, there will be no stack at all
		static void setStackSize (int stackSize);

		static void setOutFile (const std::string_view fileName); //It'll rotate by day if the template has a %d
		static void setOutPath (const std::string_view filePath);
		static void setLevelColor (LogLevel logLevel, LogColor logColor);

		static void setConsoleLevel (LogLevel logLevel);
		static void setFileLevel (LogLevel logLevel);
		static void setStackLevel (LogLevel logLevel);

		//The same as above, but allowing this: setConsoleLevel (StreamLogger::info);
		static void setConsoleLevel (BaseStreamLogger logLevel);
		static void setFileLevel (BaseStreamLogger logLevel);
		static void setStackLevel (BaseStreamLogger logLevel);
	};



	//-------------- Classes to use externally ----------------

	class LogMessageBuilder
	{
	public:
		LogMessageBuilder (BaseStreamLogger& logger);
		LogMessageBuilder (const LogMessageBuilder& other) = delete;
		LogMessageBuilder (LogMessageBuilder&& other) noexcept;
		~LogMessageBuilder ();

		LogMessageBuilder& operator<<(const std::string& value);
		LogMessageBuilder& operator<<(int value);
		LogMessageBuilder& operator<<(double value);

	private:
		bool sendToLog;
		BaseStreamLogger& logger;
		//YAGNI: consider use a stringStream, but, worth to add additioonal dependencies?
		std::string message;
	};

	class BaseStreamLogger
	{
	public:
		BaseStreamLogger (LogLevel level);

		const LogLevel level;

		LogMessageBuilder operator<<(const std::string& value);
		LogMessageBuilder operator<<(int value);
		LogMessageBuilder operator<<(double value);

		void log (const std::string& message);
	};

	extern BaseStreamLogger trace;
	extern BaseStreamLogger debug;
	extern BaseStreamLogger info;
	extern BaseStreamLogger error;
	extern BaseStreamLogger fatal;


	//YAGNI: conside move to another header file
	// We cant use std::string_view because in multi-threading, the subyacent string could be deleted
	//--- Retrieve the generated events ---
	class LogEventsReceiver
	{
	public:
		virtual void receive (const std::string& date, const std::string logTxt, LogLevel logLevel) = 0;
	};

	void retrieveLogEvents (LogEventsReceiver& receiver, BaseStreamLogger& logLevel);
	void retrieveLogEvents (LogEventsReceiver& receiver, LogLevel logLevel);

	//--- Utility Functions ---
	const std::string& getLevelName (LogLevel logLevel);


}
