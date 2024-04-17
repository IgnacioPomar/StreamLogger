# StreamLogger
StreamLogger is a modern and fast logging library designed for C++ applications. It provides a familiar syntax similar to standard C++ logging mechanisms while offering advanced features like the ability to retrieve and display log events on different devices.

## Features

- **Performance-Oriented**: Built to be fast and efficient, minimizing the overhead on your application.
- **Modern C++ Standards**: Utilizes modern C++ paradigms for ease of integration and use.
- **Cross-Device Logging**: Supports retrieving the latest log events for display on various devices, enhancing debugging and monitoring capabilities.

## TODO
When needed, i'll implement the following features:
- Multi-threading flavor
- Category classification
- File and line number
- Replace the string conversions with real streams (check wich dependencies are needed)

## Example of use

Here is a simple example demonstrating how to use StreamLogger in your application:

```cpp
#include "StreamLogger.h"

namespace lggr = StreamLogger;

class Receiver : public log::LogEventsReceiver {
public:
    void receive(const std::string &date, const std::string logTxt, lggr::LogLevel logLevel) {
        std::cout << date << " " << logTxt << " " << lggr::getLevelName(logLevel) << std::endl;
    }
};

int main ()
{
	// Default values in StreamLogger::DEFAULTS, defined in StreamLoggerConsts.h
	lggr::Config::setStackLevel (lggr::LogLevel::INFO);
	lggr::Config::setConsoleLevel (lggr::LL::TRACE);
	// lggr::Config::setFileLevel (lggr::LL::INFO);
	// lggr::Config::setOutPath ("./logs"); //Default is the executable path
	lggr::Config::setOutFile ("%d_MyLog.log");

	lggr::trace << "trace";
	lggr::debug << "debug";
	lggr::info << "info";
	lggr::warn << "warn";
	lggr::error << "error";
	lggr::fatal << "fatal";

	lggr::info << "Number: " << 33 << "\tdouble: " << .5;

	// Show events.... again
	Receiver receiver;
	retrieveLogEvents (receiver, lggr::LL::ERROR);

	return 0;
}
```
This example sets up logging levels, logs various messages, and demonstrates how to retrieve and display log events.
