#include "StreamLogger/StreamLogger.h"

namespace lggr = IgnacioPomar::Util::StreamLogger;

int main ()
{
	lggr::Config::setColorMode (lggr::ColorMode::NEVER);
	lggr::info << "StreamLogger package works";
	return 0;
}
