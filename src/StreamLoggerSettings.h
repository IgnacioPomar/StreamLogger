
#include <string>

namespace IgnacioPomar::Util::StreamLogger
{

	extern LogColor levelColors [6];

	// Pointer to the current logger
	class StackLogger;
	StackLogger &getLogger ();
}    // namespace IgnacioPomar::Util::StreamLogger
