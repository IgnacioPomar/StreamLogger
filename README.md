# StreamLogger

StreamLogger is an advanced logging library tailored for service-oriented C++ applications. It is designed to not only offer a high-performance logging mechanism but also to facilitate the integration of log data across different services and applications.
StreamLogger extends traditional logging functionalities to support event-driven architectures, allowing for logs to be pushed and accessed across various systems dynamically.

## Features

- **Service-Oriented Architecture Support**: StreamLogger is optimized for use in microservices and distributed systems where logging data needs to be propagated across different services.
- **Push Events Capability**: Enables applications to push log events to other services or applications, allowing for real-time monitoring and responsive event handling.
- **Performance-Oriented**: Built to be fast and efficient, minimizing the overhead on your application.
- **Memory Safety**: Utilizes modern C++ features, to ensure safety and robustness by avoiding the use of raw pointers.
- **Modern C++ Standards**: Utilizes modern C++ paradigms for ease of integration and use.
- **Cross-Device and Cross-Service Logging**: Provides flexible log event retrieval and display across multiple devices and services, enhancing the capability for comprehensive system diagnostics and monitoring.
- **Timed Events**: You can use timmed events.

## Wishlist
These are things that seem to me like a good idea. Not all of these are likely to be implemented without outside help, and some of them will positively never be implemented.
- Category classification
- Keep thread id if multithread

Probably impossible:
- Keep file and line number (impossible if we want to keep the stream interface)

## Build

Requirements: CMake >= 3.23, a C++20 compiler and Conan 2.

```bash
conan install . --build=missing -s compiler.cppstd=20 -o build_tests=True
cmake --preset conan-release
cmake --build --preset conan-release
ctest --preset conan-release
```

The tests use [Catch2](https://github.com/catchorg/Catch2) and [Trompeloeil](https://github.com/rollbear/trompeloeil). With `build_tests=True` the example (`examples/example.cpp`) is also built.

Without Conan, it can be added to a CMake project with `add_subdirectory` (the tests are off by default):

```cmake
add_subdirectory(StreamLogger)
target_link_libraries(myTarget PRIVATE StreamLogger::StreamLogger)
```

## Conan package

```bash
conan create . --build=missing -s compiler.cppstd=20
```

Usage from another project:

```python
self.requires("streamlogger/0.1.0")
```

```cmake
find_package(StreamLogger REQUIRED)
target_link_libraries(myTarget PRIVATE StreamLogger::StreamLogger)
```

```cpp
#include "StreamLogger/StreamLogger.h"
```

## Configuration

All the configuration is in `lggr::Config`. The default values are in `StreamLogger::DEFAULTS` (`StreamLoggerConsts.h`):

- **Multi-thread safe** by default. `Config::setMultiThreadSafe (false)` must be called before the first log.
- **Console**: the output goes to `std::clog` (stderr), from `INFO`. The colors (`Config::setColorMode`) are `AUTO` by default: only if stderr is a terminal and the [`NO_COLOR`](https://no-color.org) environment variable is not set, so journald or Docker do not get ANSI sequences.
- **File**: disabled by default. Enable it with `Config::setFileLevel`. The file name (`Config::setOutFile`) rotates each day if it has a `%d` (UTC date); the default is `%d_StreamedLog.log`. The path (`Config::setOutPath`) is the working directory by default. If the file can not be opened, the file output is disabled and an `ERROR` event is generated.
- **Stack**: the last 1000 events from `INFO` are kept in memory, to be pulled with `pullLogEvents`.
- **Push subscribers**: `subscribePushEvents` / `unsubscribePushEvents`. A subscriber must unsubscribe before being destroyed. The subscribers are called with the logger locked: they must not log.
- **Date**: always `YYYY-MM-DD HH:MM:SS.mmm UTC`, with any compiler.
- **Errors**: the errors while logging inside a destructor (the message builder, a timed event) never escape: they are reported in stderr.

## Example of use

Here is a simple example demonstrating how to use StreamLogger in your application:

```cpp
#include <iostream>
#include "StreamLogger/StreamLogger.h"

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
	lggr::Config::setMultiThreadSafe (true);    // Already the default

	PushEventHandler pushHandler;
	lggr::subscribePushEvents (pushHandler, lggr::LL::FATAL);

	// Default values in StreamLogger::DEFAULTS, defined in StreamLoggerConsts.h
	lggr::Config::setStackLevel (lggr::LogLevel::INFO);
	lggr::Config::setConsoleLevel (lggr::LL::TRACE);
	lggr::Config::setFileLevel (lggr::LL::INFO);    // Default is OFF: no log file
	// lggr::Config::setOutPath ("./logs"); //Default is the working directory
	// lggr::Config::setColorMode (lggr::ColorMode::NEVER); //Default is AUTO: only in a terminal
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

	lggr::unsubscribePushEvents (pushHandler);

	return 0;
}
```
This example sets up logging levels, logs various messages, and demonstrates how to retrieve and display log events.

This will output something similar to (with colors):
```	
2024-04-17 15:28:07.212 UTC [TRACE]     0       Song: London Bridge is falling down
2024-04-17 15:28:07.214 UTC [DEBUG]     1       Falling down, falling down
2024-04-17 15:28:07.215 UTC [DEBUG]     2       London Bridge is falling down
2024-04-17 15:28:07.215 UTC [DEBUG]     3       My fair lady
2024-04-17 15:28:07.215 UTC [INFO]      4       Build it up with iron bars
2024-04-17 15:28:07.216 UTC [INFO]      5       Iron bars, iron bars
2024-04-17 15:28:07.216 UTC [INFO]      6       Build it up with iron bars
2024-04-17 15:28:07.217 UTC [DEBUG]     7       My fair lady
2024-04-17 15:28:07.217 UTC [WARN]      8       Iron bars will bend and break
2024-04-17 15:28:07.218 UTC [WARN]      9       Bend and break, bend and break
2024-04-17 15:28:07.219 UTC [WARN]      10      Iron bars will bend and break
2024-04-17 15:28:07.220 UTC [DEBUG]     11      My fair lady
2024-04-17 15:28:07.220 UTC [INFO]      12      Build it up with silver and gold
2024-04-17 15:28:07.221 UTC [INFO]      13      Silver and gold, silver and gold
2024-04-17 15:28:07.221 UTC [INFO]      14      Build it up with silver and gold
2024-04-17 15:28:07.222 UTC [DEBUG]     15      My fair lady
2024-04-17 15:28:07.222 UTC [INFO]      16      Set a man to watch all night
2024-04-17 15:28:07.223 UTC [ERROR]     17      Suppose the man should fall asleep
2024-04-17 15:28:07.223 UTC [FATAL]     18      The man finally fell asleep, the man finally fell asleep
***** PUSH EVENT *****          FATAL           18      The man finally fell asleep, the man finally fell asleep
2024-04-17 15:28:07.224 UTC [ERROR]     19      fell asleep,  fell asleep
2024-04-17 15:28:07.224 UTC [FATAL]     There is no bridge left! the thieves stole it!!
***** PUSH EVENT *****          FATAL           There is no bridge left! the thieves stole it!!
>>> EVENT Pulled >>>    2024-04-17 15:28:07.223 UTC [ERROR]     17      Suppose the man should fall asleep
>>> EVENT Pulled >>>    2024-04-17 15:28:07.223 UTC [FATAL]     18      The man finally fell asleep, the man finally fell asleep
>>> EVENT Pulled >>>    2024-04-17 15:28:07.224 UTC [ERROR]     19      fell asleep,  fell asleep
>>> EVENT Pulled >>>    2024-04-17 15:28:07.224 UTC [FATAL]     There is no bridge left! the thieves stole it!!
```



## License
The StreamLogger library is licensed under the Unlicense. See the LICENSE file for more information.
