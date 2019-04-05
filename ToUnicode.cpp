#include "ToUnicode.h"
#include "SDL.h"
#include <iostream>
#include <cwchar>

std::wstring MultibyteToWideString(const char* source)
{
	std::wstring result;

	const char* p = source;
	size_t messageCharCount = 0;

	// calculate the number of wide characters needed (also checks encoding validity)
	{
		auto mbstate = std::mbstate_t();
		messageCharCount = std::mbsrtowcs(nullptr, &p, 0, &mbstate);
		if (messageCharCount == size_t(-1)) {
			SDL_SetError("Invalid character sequence");
			return result;
		}
	}	

	result.resize(messageCharCount);
	{
		auto mbstate = std::mbstate_t();
		p = source;
		std::mbsrtowcs(const_cast<wchar_t*>(result.data()), &p, messageCharCount, &mbstate);
	}
	return result;
}
