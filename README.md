# StreamLogger

StreamLogger is an advanced logging library tailored for service-oriented C++ applications. It is designed to not only offer a high-performance logging mechanism but also to facilitate the integration of log data across different services and applications.
StreamLogger extends traditional logging functionalities to support event-driven architectures, allowing for logs to be pushed and accessed across various systems dynamically.

## Features

- **Service-Oriented Architecture Support**: StreamLogger is optimized for use in microservices and distributed systems where logging data needs to be propagated across different services.
- **Push Events Capability**: Enables applications to push log events to other services or applications, allowing for real-time monitoring and responsive event handling.
- **Performance-Oriented**: Built to be fast and efficient, minimizing the overhead on your application.
- **Modern C++ Standards**: Utilizes modern C++ paradigms for ease of integration and use.
- **Cross-Device and Cross-Service Logging**: Provides flexible log event retrieval and display across multiple devices and services, enhancing the capability for comprehensive system diagnostics and monitoring.

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

namespace lggr = IgnacioPomar::Util::StreamLogger;

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
