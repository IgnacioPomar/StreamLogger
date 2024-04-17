#include "StackLogger.h"
#include "StreamLogger.h"

namespace IgnacioPomar::Util::StreamLogger
{
	/**
	 * Constructor
	 */
	LogMessageBuilder::LogMessageBuilder (BaseStreamLogger &logger)
	    : logger (logger)
	{
	}

	/**
	 * Move constructor
	 */
	LogMessageBuilder::LogMessageBuilder (LogMessageBuilder &&other) noexcept
	    : logger (other.logger)
	    , message (std::move (other.message))
	{
	}

	/**
	 * Destructor, log the message
	 */
	LogMessageBuilder::~LogMessageBuilder()
	{
		// If using string stream
		// logger.log (stream.str ());

		// As long as message moves to child, we only send it if it is not empty
		if (!message.empty())
		{
			logger.log (message);
		}
	}

	LogMessageBuilder &LogMessageBuilder::operator<< (const std::string &s)
	{
		this->message.append (s);
		return *this;
	}
	LogMessageBuilder &LogMessageBuilder::operator<< (int value)
	{
		this->message.append (std::to_string (value));
		return *this;
	}
	LogMessageBuilder &LogMessageBuilder::operator<< (double value)
	{
		this->message.append (std::to_string (value));
		return *this;
	}
}    // namespace IgnacioPomar::Util::StreamLogger
