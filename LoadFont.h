#pragma once

#include <vector>

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
	bool GetGlyphRect(int charCode, SDL_Rect& glyphRect) const;
	bool GetGlyphGeometry(int charCode, stbtt_packedchar &glyphGeometry) const;

	/**
	 * Returns a pointer to the internal surface that holds the glyphs.
	 * Use GetGlyphGeometry() to find out coordinates of a glyph image in this surface.
	 */
	SDL_Surface* GetSurface() { return fontSurface; }

	SDL_Rect ComputeTextSize(const std::vector<wchar_t> &text);

private:

	bool ok = false;
	SDL_Surface* fontSurface = nullptr;
	stbtt_fontinfo fontInfo = { 0 };
	stbtt_packedchar packedChars[NUMBER_OF_CHARS];
};
