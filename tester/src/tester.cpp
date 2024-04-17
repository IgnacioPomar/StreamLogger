#include <iostream>
#include "StreamLogger.h"

#ifdef _DEBUG
#	define END_LIB_STD "d.lib"
#else
#	define END_LIB_STD ".lib"
#endif

#pragma comment(lib, "StreamLogger" END_LIB_STD)

namespace lggr = IgnacioPomar::Util::StreamLogger;

class EventReprinter : public lggr::LogEventsSubscriber
{
	public:
		void onLogEvent (const std::string &date, const std::string logTxt, lggr::LogLevel logLevel) const
		{
			// Parse as json, or save to a database, or whatever action you want
			std::cout << ">>> EVENT Pulled >>>\t" << date << " [" << lggr::getLevelName (logLevel) << "]\t" << logTxt
			          << std::endl;
		}
};

class PushEventHandler : public lggr::LogEventsSubscriber
{
	public:
		void onLogEvent (const std::string &date, const std::string logTxt, lggr::LogLevel logLevel) const
		{
			// Use a web service, or call a function, or call a script, or whatever action you want
			std::cout << "***** PUSH EVENT ***** \t\t" << lggr::getLevelName (logLevel) << "\t\t" << logTxt
			          << std::endl;
		}
};

int main ()
{
	PushEventHandler pushHandler;
	lggr::subscribePushEvents (pushHandler, lggr::LL::FATAL);

	// Default values in StreamLogger::DEFAULTS, defined in StreamLoggerConsts.h
	lggr::Config::setStackLevel (lggr::LogLevel::INFO);
	lggr::Config::setConsoleLevel (lggr::LL::TRACE);
	// lggr::Config::setFileLevel (lggr::LL::INFO);
	// lggr::Config::setOutPath ("./logs"); //Default is the executable path
	lggr::Config::setOutFile ("%d_MyLog.log");

	lggr::trace << "Song: London Bridge is falling down";
	lggr::debug << "Falling down, falling down";
	lggr::debug << "London Bridge is falling down";
	lggr::debug << "My fair lady";
	lggr::info << "Build it up with iron bars";
	lggr::info << "Iron bars, iron bars";
	lggr::info << "Build it up with iron bars";
	lggr::debug << "My fair lady";
	lggr::warn << "Iron bars will bend and break";
	lggr::warn << "Bend and break, bend and break";
	lggr::warn << "Iron bars will bend and break";
	lggr::debug << "My fair lady";
	lggr::info << "Build it up with silver and gold";
	lggr::info << "Silver and gold, silver and gold";
	lggr::info << "Build it up with silver and gold";
	lggr::debug << "My fair lady";
	lggr::info << "Set a man to watch all night";
	lggr::error << "Suppose the man should fall asleep";
	lggr::fatal << "The man finally slept, the man finally slept";
	lggr::fatal << "... the bridge is broken ...";

	lggr::info << "Number: " << 33 << "\tdouble: " << .5;

	// Show events.... again
	EventReprinter reprinter;
	pullLogEvents (reprinter, lggr::LL::ERROR);

	return 0;
}
