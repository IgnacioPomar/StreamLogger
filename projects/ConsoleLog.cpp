

#include "StreamLogger.h"


//Windows.h must be in the last as ERROR is redefined
#ifdef _WIN32
#include <Windows.h>

//Define extra colours
#define FOREGROUND_WHITE		(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW       	(FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_CYAN		        (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_MAGENTA	        (FOREGROUND_RED | FOREGROUND_BLUE)
#define FOREGROUND_BLACK		0x0000

#define FOREGROUND_INTENSE_RED		(FOREGROUND_RED | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_GREEN	(FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_BLUE		(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_WHITE	(FOREGROUND_WHITE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_YELLOW	(FOREGROUND_YELLOW | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_CYAN		(FOREGROUND_CYAN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_MAGENTA	(FOREGROUND_MAGENTA | FOREGROUND_INTENSITY)


#define BACKGROUND_WHITE		(BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN)
#define BACKGROUND_YELLOW	        (BACKGROUND_RED | BACKGROUND_GREEN)
#define BACKGROUND_CYAN		        (BACKGROUND_BLUE | BACKGROUND_GREEN)
#define BACKGROUND_MAGENTA	        (BACKGROUND_RED | BACKGROUND_BLUE)
#define BACKGROUND_BLACK		0x0000

#define BACKGROUND_INTENSE_RED		(BACKGROUND_RED | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_GREEN	(BACKGROUND_GREEN | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_BLUE		(BACKGROUND_BLUE | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_WHITE	(BACKGROUND_WHITE | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_YELLOW	(BACKGROUND_YELLOW | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_CYAN		(BACKGROUND_CYAN | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_MAGENTA	(BACKGROUND_MAGENTA | BACKGROUND_INTENSITY)

#endif

namespace StreamLogger
{

	HANDLE GetConsoleHandle()
	{
		static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		return hConsole;
	}

	void setConsoleColor(LogColor color)
	{
		switch (color)
		{
		case LogColor::BLACK:
			SetConsoleTextAttribute(GetConsoleHandle(), 0x0000);
			break;
		case LogColor::WHITE:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_INTENSE_WHITE);
			break;
		case LogColor::GREY:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_WHITE);
			break;
		case LogColor::RED:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_RED);
			break;
		case LogColor::LIGHTRED:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_INTENSE_RED);
			break;
		case LogColor::GREEN:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_GREEN);
			break;
		case LogColor::YELLOW:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_INTENSE_YELLOW);
			break;
		case LogColor::BLUE:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_BLUE);
			break;
		case LogColor::MAGENTA:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_MAGENTA);
			break;
		case LogColor::CYAN:
			SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_INTENSE_CYAN);
			break;
		default:
			break;
		}

	}
	void resetConsoleColor()
	{
		SetConsoleTextAttribute(GetConsoleHandle(), FOREGROUND_WHITE);
	}
} // namespace StreamLogger
