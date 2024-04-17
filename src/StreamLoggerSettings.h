
#include <string>

namespace StreamLogger
{

	extern LogColor levelColors [6];

	// Pointer to the current logger
	class StackLogger;
	StackLogger &getLogger ();
}    // namespace StreamLogger
