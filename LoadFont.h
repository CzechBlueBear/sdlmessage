#pragma once

#include <string>
#include <memory>
#include <vector>

#include "SDLWrapper.h"
#include "MapFile.h"

#include "stb_truetype.h"

class Font {
public:

	static const uint32_t kCharsetLatin = 0x1;
	static const uint32_t kCharsetCyrillic = 0x2;
	static const uint32_t kCharsetGreek = 0x4;

	Font(const MappedFile &fontFile, float fontSize, uint32_t extraCharsetSupport = 0);
	~Font();
	bool Ok() const { return ok; }
	bool GetGlyphRect(int charCode, SDL_Rect& glyphRect) const;
	bool GetGlyphGeometry(int charCode, stbtt_packedchar &glyphGeometry) const;

	/// Returns the internal surface that holds the glyphs.
	/// Use GetGlyphGeometry() to find out coordinates of a glyph image in this surface.
	SDL::Surface& GetSurface() { return *(fontSurface.get()); }

	SDL_Rect ComputeTextSize(const std::wstring &text);

private:

	const stbtt_packedchar* GetPackedChar(int charCode) const;

	uint32_t encodedCharsets = 0;
	bool ok = false;
	int encodedCharCount = 0;
	std::unique_ptr<SDL::Surface> fontSurface = nullptr;
	stbtt_fontinfo fontInfo = { 0 };
	std::vector<stbtt_pack_range> packedCharRanges;
	std::array<stbtt_packedchar, 767> packedCharsBasic;
	std::array<stbtt_packedchar, 256> packedCharsCyrillic;
	std::array<stbtt_packedchar, 143> packedCharsGreek;
};
