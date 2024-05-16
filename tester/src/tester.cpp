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
		void onLogEvent (const std::string &date, const std::string logTxt, lggr::LogLevel logLevel)
		{
			// Parse as json, or save to a database, or whatever action you want
			std::cout << ">>> EVENT Pulled >>>\t" << date << " [" << lggr::getLevelName (logLevel) << "]\t" << logTxt
			          << std::endl;
		}
};

class PushEventHandler : public lggr::LogEventsSubscriber
{
	public:
		void onLogEvent (const std::string &date, const std::string logTxt, lggr::LogLevel logLevel)
		{
			// Use a web service, or call a function, or call a script, or whatever action you want
			std::cout << "***** PUSH EVENT ***** \t\t" << lggr::getLevelName (logLevel) << "\t\t" << logTxt
			          << std::endl;
		}
};

int main ()
{
	lggr::Config::setMultiThreadSafe (true);

	PushEventHandler pushHandler;
	lggr::subscribePushEvents (pushHandler, lggr::LL::FATAL);

	// Default values in StreamLogger::DEFAULTS, defined in StreamLoggerConsts.h
	lggr::Config::setStackLevel (lggr::LogLevel::INFO);
	lggr::Config::setConsoleLevel (lggr::LL::TRACE);
	// lggr::Config::setFileLevel (lggr::LL::INFO);
	// lggr::Config::setOutPath ("./logs"); //Default is the executable path
	lggr::Config::setOutFile ("%d_MyLog.log");

	int line = 0;

	lggr::trace << line++ << "\tSong: London Bridge is falling down";
	lggr::debug << line++ << "\tFalling down, falling down";
	lggr::debug << line++ << "\tLondon Bridge is falling down";
	lggr::debug << line++ << "\tMy fair lady";

	{
		auto timedEvt = lggr::info.startTimedEvent();
		timedEvt << line++ << "\tBuild it up with iron bars";
		lggr::info << line++ << "\tIron bars, iron bars";
		lggr::info << line++ << "\tBuild it up with iron bars";
		lggr::debug << line++ << "\tMy fair lady";
	}

	lggr::warn << line++ << "\tIron bars will bend and break";
	lggr::warn << line++ << "\tBend and break, bend and break";
	lggr::warn << line++ << "\tIron bars will bend and break";
	lggr::debug << line++ << "\tMy fair lady";
	lggr::info << line++ << "\tBuild it up with silver and gold";
	lggr::info << line++ << "\tSilver and gold, silver and gold";
	lggr::info << line++ << "\tBuild it up with silver and gold";
	lggr::debug << line++ << "\tMy fair lady";
	lggr::info << line++ << "\tSet a man to watch all night";
	lggr::error << line++ << "\tSuppose the man should fall asleep";
	lggr::fatal << line++ << "\tThe man finally fell asleep, the man finally fell asleep";
	lggr::error << line++ << "\tfell asleep,  fell asleep";
	lggr::fatal << "There is no bridge left! the thieves stole it!!";

	// Second TEST: ONLY subscriber events
	lggr::Config::setFileLevel (lggr::LL::OFF);
	lggr::Config::setStackLevel (lggr::LL::OFF);
	lggr::Config::setConsoleLevel (lggr::LL::OFF);

	lggr::info << "This message will be ignored";
	lggr::fatal << "This message will be shown only as push";

	// Show events.... again (except the last one)
	EventReprinter reprinter;
	lggr::pullLogEvents (reprinter, lggr::LL::INFO);

	return 0;
}
