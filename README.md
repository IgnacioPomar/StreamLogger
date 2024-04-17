# StreamLogger
StreamLogger is a modern and fast logging library designed for C++ applications. It provides a familiar syntax similar to standard C++ logging mechanisms while offering advanced features like the ability to retrieve and display log events on different devices.

## Features

- **Performance-Oriented**: Built to be fast and efficient, minimizing the overhead on your application.
- **Modern C++ Standards**: Utilizes modern C++ paradigms for ease of integration and use.
- **Cross-Device Logging**: Supports retrieving the latest log events for display on various devices, enhancing debugging and monitoring capabilities.

## Example of use

Here is a simple example demonstrating how to use StreamLogger in your application:

```cpp
#include "StreamLogger.h"

namespace log = StreamLogger;

class Receiver : public log::LogEventsReceiver {
public:
    void receive(const std::string &date, const std::string logTxt, lggr::LogLevel logLevel) {
        std::cout << date << " " << logTxt << " " << lggr::getLevelName(logLevel) << std::endl;
    }
};

int main() {
    log::Config::setStackLevel  (log::LogLevel::INFO);
    log::Config::setConsoleLevel(log::LL::TRACE); //Using a alias

    log::trace << "trace";
    log::debug << "debug";
    log::info << "info";
    log::warn << "warn";
    log::error << "error";
    log::fatal << "fatal";

    log::info << "Number: " << 33 << "\tdouble: " << .5;

    // Show events again
    Receiver receiver;
    retrieveLogEvents(receiver, lggr::LL::ERROR);

    return 0;
}
```
This example sets up logging levels, logs various messages, and demonstrates how to retrieve and display log events.
