#include <iostream>
#include "StreamLogger.h"



#ifdef _DEBUG 
#define END_LIB_STD "d.lib"
#else
#define END_LIB_STD ".lib"
#endif

#pragma comment(lib, "StreamLogger" END_LIB_STD)



namespace lggr = StreamLogger;

class Receiver : public lggr::LogEventsReceiver
{
public:
	void receive (const std::string& date, const std::string logTxt, lggr::LogLevel logLevel)
	{
		std::cout << date << " " << logTxt << " " << lggr::getLevelName (logLevel) << std::endl;
	}

};


int main ()
{
	int i = 55;

	lggr::Config::setStackLevel (lggr::info);
	lggr::Config::setConsoleLevel (lggr::trace);

	lggr::trace << "trace";
	lggr::debug << "debug";
	lggr::info << "info";
	lggr::warn << "warn";
	lggr::error << "error";
	lggr::fatal << "fatal";

	lggr::info << "Number: " << 33 << "\tdouble: " << .5;

	//Show events.... again
	Receiver receiver;
	retrieveLogEvents (receiver, lggr::debug);

	return 0;
}
