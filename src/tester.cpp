#include <iostream>
#include "StreamLogger.h"

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


	lggr::info << "Number: " << 33 << "\tdouble: " << .5;

	//Show events.... again
	Receiver receiver;
	retrieveLogEvents (receiver, lggr::debug);

	return 0;
}
