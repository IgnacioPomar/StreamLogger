/*********************************************************************************************
 * Description  : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <string>

namespace IgnacioPomar::Util::StreamLogger
{
	// Pointer to the current logger
	class StackLogger;
	StackLogger &getLogger ();
}    // namespace IgnacioPomar::Util::StreamLogger
