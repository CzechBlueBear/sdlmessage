#pragma once

#include "SDL.h"
#include "MapFile.h"

#include "stb_truetype.h"

class Font {
public:

	static const int NUMBER_OF_CHARS = 0x1ff;
	static const int DEFAULT_FONT_SURFACE_WIDTH = 2048;
	static const int DEFAULT_FONT_SURFACE_HEIGHT = 512;

	Font(const MappedFile &fontFile, float fontSize);
	~Font();
	bool Ok() const { return ok; }
	SDL_Rect GetGlyphRect(int charCode) const;
	SDL_Surface* GetSurface() { return fontSurface; }

private:

	bool ok = false;
	SDL_Surface* fontSurface = nullptr;
	stbtt_fontinfo fontInfo = { 0 };
	stbtt_packedchar packedChars[NUMBER_OF_CHARS];
};
