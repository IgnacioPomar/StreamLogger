
#include <string>

namespace StreamLogger
{

	extern bool rotateByDay;
	extern std::string logPath;
	extern std::string logName;

	extern int stackSize;

	extern LogColor levelColors [6];

	extern LogLevel consoleLevel;
	extern LogLevel fileLevel;
	extern LogLevel stackLevel;

	// Pointer to the current logger
	class StackLogger;
	StackLogger &getLogger ();
}    // namespace StreamLogger
