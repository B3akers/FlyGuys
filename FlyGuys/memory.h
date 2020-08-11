#pragma once
#include <Windows.h>
#include <inttypes.h>
namespace memory
{
	extern char* find_text_pattern(const char* szModule, const char* szSignature);
	extern uint8_t* find_signature(const char* szModule, const char* szSignature);
};