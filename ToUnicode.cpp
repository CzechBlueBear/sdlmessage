#include "ToUnicode.h"
#include "SDL.h"
#include <iostream>

bool StringToUnicode(const char* text, std::vector<wchar_t> &result)
{
	result.clear();

	const char* p = text;
	size_t messageCharCount = 0;

	{
		mbstate_t mbstate = { 0 };
		messageCharCount = mbsrtowcs(nullptr, &p, 0, &mbstate);
		if (messageCharCount == size_t(-1)) {
			SDL_SetError("Invalid character sequence");
			return false;
		}
	}

	{
		result.resize(messageCharCount);
		mbstate_t mbstate = { 0 };
		p = text;
		mbsrtowcs(result.data(), &p, messageCharCount, &mbstate);
	}

	return true;
}
