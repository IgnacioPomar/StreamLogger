/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include "StreamLogger/StreamLogger.h"
#include "StreamLogger/StreamLoggerVersion.h"

namespace IgnacioPomar::Util::StreamLogger
{
	// The macros are expanded here, when the library is compiled: the values are the ones of the loaded library
	const LibraryInfo &getLibraryInfo () noexcept
	{
		static constexpr LibraryInfo libraryInfo {
		    STREAMLOGGER_NAME,          STREAMLOGGER_DESCRIPTION,   STREAMLOGGER_VERSION,
		    STREAMLOGGER_VERSION_MAJOR, STREAMLOGGER_VERSION_MINOR, STREAMLOGGER_VERSION_PATCH,
		};
		return libraryInfo;
	}
}    // namespace IgnacioPomar::Util::StreamLogger
