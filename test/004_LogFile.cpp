/*********************************************************************************************
 *  Description : Modern C++ logger library, with evernt retrieval and color support
 *  License     : The unlicense (https://unlicense.org)
 *	Copyright	(C) 2024  Ignacio Pomar Ballestero
 ********************************************************************************************/

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

#include "TestUtils.h"

namespace fs = std::filesystem;

namespace
{
	// Temporal directory, removed at the end of the test
	class TmpDir
	{
		public:
			TmpDir()
			{
				std::random_device rd;
				path = fs::temp_directory_path() / ("StreamLoggerTest_" + std::to_string (rd()));
				fs::create_directories (path);
			}
			~TmpDir()
			{
				std::error_code ec;
				fs::remove_all (path, ec);
			}
			fs::path path;
	};

	std::string readFile (const fs::path &file)
	{
		std::ifstream in (file);
		std::stringstream ss;
		ss << in.rdbuf();
		return ss.str();
	}

	// The file name uses the UTC date of the event
	std::string today ()
	{
		auto ymd = std::chrono::year_month_day {floor<std::chrono::days> (std::chrono::system_clock::now())};
		char buf [16];
		std::snprintf (buf, sizeof (buf), "%04d-%02u-%02u", int (ymd.year()), unsigned (ymd.month()),
		               unsigned (ymd.day()));
		return buf;
	}
}    // namespace

TEST_CASE ("The log file name has the date", "[file]")
{
	TmpDir tmp;
	lggr::StackLogger logger;
	silence (logger);
	logger.setOutPath (tmp.path.string());
	logger.setOutFile ("%d_MyLog.log");
	logger.setFileLevel (lggr::LL::INFO);

	log (logger, lggr::LL::INFO, "First line");

	fs::path expected = tmp.path / (today() + "_MyLog.log");
	REQUIRE (fs::exists (expected));
	CHECK (readFile (expected).find ("[INFO]\tFirst line\n") != std::string::npos);

	SECTION ("The pattern survives the rotation")
	{
		// Simulate a day change
		logger.lastLogDate = std::chrono::year_month_day {std::chrono::year (2000), std::chrono::January,
		                                                  std::chrono::day (1)};
		log (logger, lggr::LL::INFO, "Second line");

		CHECK (logger.logFilePattern == "%d_MyLog.log");
		CHECK (logger.hasRotation);
		CHECK (readFile (expected).find ("[INFO]\tSecond line\n") != std::string::npos);
	}
}

TEST_CASE ("A log file without date", "[file]")
{
	TmpDir tmp;
	lggr::StackLogger logger;
	silence (logger);
	logger.setOutPath (tmp.path.string());
	logger.setOutFile ("fixed.log");
	logger.setFileLevel (lggr::LL::WARN);

	log (logger, lggr::LL::INFO, "Not in the file");
	log (logger, lggr::LL::WARN, "In the file");

	std::string content = readFile (tmp.path / "fixed.log");
	CHECK (content.find ("Not in the file") == std::string::npos);
	CHECK (content.find ("[WARN]\tIn the file\n") != std::string::npos);
	CHECK_FALSE (logger.hasRotation);
}

TEST_CASE ("Changing the path opens a new file", "[file]")
{
	TmpDir first;
	TmpDir second;
	lggr::StackLogger logger;
	silence (logger);
	logger.setOutFile ("app.log");
	logger.setFileLevel (lggr::LL::INFO);

	logger.setOutPath (first.path.string());
	log (logger, lggr::LL::INFO, "One");
	logger.setOutPath (second.path.string());
	log (logger, lggr::LL::INFO, "Two");

	CHECK (readFile (first.path / "app.log").find ("Two") == std::string::npos);
	CHECK (readFile (second.path / "app.log").find ("Two") != std::string::npos);
}

TEST_CASE ("There is no log file by default", "[file]")
{
	lggr::StackLogger logger;
	CHECK (logger.fileLevel == lggr::LL::OFF);
}

TEST_CASE ("A log file that can not be opened disables the file output", "[file]")
{
	// The MT safe logger used to deadlock here
	lggr::StackLoggerMTSafe logger;
	silence (logger);
	logger.setOutPath ("/nonexistent/StreamLogger/dir");
	logger.setFileLevel (lggr::LL::INFO);

	log (logger, lggr::LL::INFO, "Lost in the file");

	CHECK (logger.fileLevel == lggr::LL::OFF);
	auto texts = pullTexts (logger, lggr::LL::TRACE);
	REQUIRE (texts.size() == 2);
	CHECK (texts [0] == "Lost in the file");
	CHECK (texts [1].starts_with ("Unable to open log file: /nonexistent/StreamLogger/dir"));
}
