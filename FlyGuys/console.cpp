#include "console.h"
#include <Windows.h>
#include <time.h>

void console::alloc()
{
	AllocConsole();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	SetConsoleTitle("Fall Guys");
}

void console::clean()
{
	FreeConsole();
}

void console::log(const char* fmt, ...)
{
	va_list va_alist;
	char logBuf[1024] = { 0 };

	va_start(va_alist, fmt);
	_vsnprintf(logBuf + strlen(logBuf), sizeof(logBuf) - strlen(logBuf), fmt, va_alist);
	va_end(va_alist);

	if (logBuf[0] != '\0')
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (FOREGROUND_GREEN | FOREGROUND_INTENSITY));
		printf("[%s]", get_time_string().c_str());
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN));
		printf(": %s\n", logBuf);
	}
}

std::string console::get_time_string()
{
	time_t current_time;
	struct tm* time_info;
	static char timeString[10];

	time(&current_time);
	time_info = localtime(&current_time);

	strftime(timeString, sizeof(timeString), "%I:%M %p", time_info);
	return timeString;
}