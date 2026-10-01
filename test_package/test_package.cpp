#include <iostream>
#include "StreamLogger/StreamLogger.h"

namespace lggr = IgnacioPomar::Util::StreamLogger;

int main ()
{
	const lggr::LibraryInfo &info = lggr::getLibraryInfo();
	std::cout << info.name << " " << info.version << " (compiled with " << STREAMLOGGER_VERSION
	          << "): " << info.description << std::endl;

	lggr::Config::setColorMode (lggr::ColorMode::NEVER);
	lggr::info << "StreamLogger package works";
	return 0;
}
