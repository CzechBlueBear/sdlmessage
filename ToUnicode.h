#pragma once
#include <vector>
#include <string>

/**
 * Converts a string from multibyte encoding to wide string
 * (under normal conditions, it is from utf-8 to Unicode).
 * \return The new wide string.
 * \bug If an error occurs, an empty string is returned,
 * and SDL_Error is set; however, without querying SDL_Error,
 * there is no way to tell if that happened.
 */
std::wstring MultibyteToWideString(const char* source);
