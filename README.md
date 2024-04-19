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
- Replace the string conversions with real streams (check wich dependencies are needed)
- Keep thread id if multithread

Probably impossible:
- Keep file and line number (impossible if we want to keep the stream interface)

## Example of use

Here is a simple example demonstrating how to use StreamLogger in your application:

```cpp
#include "StreamLogger.h"

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

	// Show events.... again
	EventReprinter reprinter;
	pullLogEvents (reprinter, lggr::LL::ERROR);

	return 0;
}
```
This example sets up logging levels, logs various messages, and demonstrates how to retrieve and display log events.

This will output something similar to (with colors):
```	
2024-04-17 15:28:07.2128489 [TRACE]     0       Song: London Bridge is falling down
2024-04-17 15:28:07.2142945 [DEBUG]     1       Falling down, falling down
2024-04-17 15:28:07.2152041 [DEBUG]     2       London Bridge is falling down
2024-04-17 15:28:07.2155484 [DEBUG]     3       My fair lady
2024-04-17 15:28:07.2159025 [INFO]      4       Build it up with iron bars
2024-04-17 15:28:07.2163818 [INFO]      5       Iron bars, iron bars
2024-04-17 15:28:07.2167742 [INFO]      6       Build it up with iron bars
2024-04-17 15:28:07.2171096 [DEBUG]     7       My fair lady
2024-04-17 15:28:07.2174377 [WARN]      8       Iron bars will bend and break
2024-04-17 15:28:07.2182144 [WARN]      9       Bend and break, bend and break
2024-04-17 15:28:07.2195249 [WARN]      10      Iron bars will bend and break
2024-04-17 15:28:07.2200839 [DEBUG]     11      My fair lady
2024-04-17 15:28:07.2206127 [INFO]      12      Build it up with silver and gold
2024-04-17 15:28:07.2212500 [INFO]      13      Silver and gold, silver and gold
2024-04-17 15:28:07.2217964 [INFO]      14      Build it up with silver and gold
2024-04-17 15:28:07.2222301 [DEBUG]     15      My fair lady
2024-04-17 15:28:07.2227241 [INFO]      16      Set a man to watch all night
2024-04-17 15:28:07.2231234 [ERROR]     17      Suppose the man should fall asleep
2024-04-17 15:28:07.2235053 [FATAL]     18      The man finally fell asleep, the man finally fell asleep
***** PUSH EVENT *****          FATAL           18      The man finally fell asleep, the man finally fell asleep
2024-04-17 15:28:07.2240149 [ERROR]     19      fell asleep,  fell asleep
2024-04-17 15:28:07.2243657 [FATAL]     There is no bridge left! the thieves stole it!!
***** PUSH EVENT *****          FATAL           There is no bridge left! the thieves stole it!!
>>> EVENT Pulled >>>    2024-04-17 15:28:07.2231234 [ERROR]     17      Suppose the man should fall asleep
>>> EVENT Pulled >>>    2024-04-17 15:28:07.2235053 [FATAL]     18      The man finally fell asleep, the man finally fell asleep
>>> EVENT Pulled >>>    2024-04-17 15:28:07.2240149 [ERROR]     19      fell asleep,  fell asleep
>>> EVENT Pulled >>>    2024-04-17 15:28:07.2243657 [FATAL]     There is no bridge left! the thieves stole it!!
```



## License
The StreamLogger library is licensed under the Unlicense. See the LICENSE file for more information.
