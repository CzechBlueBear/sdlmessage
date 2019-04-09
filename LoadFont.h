#pragma once

#include <string>
#include <memory>

#include "SDLWrapper.h"
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

	/// Returns the internal surface that holds the glyphs.
	/// Use GetGlyphGeometry() to find out coordinates of a glyph image in this surface.
	SDL::Surface& GetSurface() { return *(fontSurface.get()); }

	SDL_Rect ComputeTextSize(const std::wstring &text);

private:

	bool ok = false;
	std::unique_ptr<SDL::Surface> fontSurface = nullptr;
	stbtt_fontinfo fontInfo = { 0 };
	stbtt_packedchar packedChars[NUMBER_OF_CHARS];
};
