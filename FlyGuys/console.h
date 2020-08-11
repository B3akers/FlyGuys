#pragma once
#include <string>
class console
{
	static std::string get_time_string();
public:
	static void alloc();
	static void clean();
	static void log(const char* fmt, ...);
};