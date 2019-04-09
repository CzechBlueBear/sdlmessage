#include "LoadFont.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <iostream>

Font::Font(const MappedFile &fontFile, float fontSize)
{
	fontSurface = std::make_unique<SDL::Surface>(
		DEFAULT_FONT_SURFACE_WIDTH, DEFAULT_FONT_SURFACE_HEIGHT,
		8, SDL_PIXELFORMAT_INDEX8
	);
	if (!fontSurface->Ok()) {
		SDL_SetError("Could not create surface: %s", SDL_GetError());
		return;
	}

	SDL_Color colorRamp[256];
	for (int i = 0; i < 256; i++) {
		colorRamp[i].r = i;
		colorRamp[i].g = i;
		colorRamp[i].b = i;
		colorRamp[i].a = 255;
	}
	SDL_SetPaletteColors(fontSurface->GetFormat()->palette, colorRamp, 0, 256);

	if (!stbtt_InitFont(&fontInfo, fontFile.GetData(), 0)) { /*stbtt_GetFontOffsetForIndex(fontFile.GetData(), 0) */
		SDL_SetError("stbtt_InitFont() failed");
		return;
	}

	stbtt_pack_context packContext = { 0 };
	if (!stbtt_PackBegin(
		&packContext,
		static_cast<uint8_t*>(fontSurface->GetPixels()),
		fontSurface->GetWidth(), fontSurface->GetHeight(), fontSurface->GetPitch(),
		1, nullptr)
	) {
		SDL_SetError("stbtt_PackBegin() failed");
		return;
	}

	if (!stbtt_PackFontRange(&packContext, fontFile.GetData(), 0, fontSize,
		0x0000, NUMBER_OF_CHARS,
		packedChars)
	) {
		SDL_SetError("stbtt_PackFontRange() failed");
		stbtt_PackEnd(&packContext);
		return;
	}

	stbtt_PackEnd(&packContext);

	ok = true;
}

Font::~Font()
{
	ok = false;
}

bool Font::GetGlyphRect(int charCode, SDL_Rect& result) const
{
	if (charCode > NUMBER_OF_CHARS) return false;

	const stbtt_packedchar& packedChar = packedChars[charCode];
	result.x = packedChar.x0;
	result.y = packedChar.y0;
	result.w = packedChar.x1 - packedChar.x0;
	result.h = packedChar.y1 - packedChar.y0;
	return true;
}

bool Font::GetGlyphGeometry(int charCode, stbtt_packedchar &glyphGeometry) const
{
	if (charCode > NUMBER_OF_CHARS) return false;
	glyphGeometry = packedChars[charCode];
	return true;
}

SDL_Rect Font::ComputeTextSize(const std::wstring &text)
{
	int x = 0, maxY = 0;
	for (wchar_t c : text) {
		stbtt_packedchar glyphGeometry;
		if (GetGlyphGeometry(int(c), glyphGeometry)) {
			x += glyphGeometry.xadvance;
			if (glyphGeometry.y1 - glyphGeometry.y0 > maxY) {
				maxY = glyphGeometry.y1 - glyphGeometry.y0;
			}
		}
	}

	SDL_Rect result;
	result.x = 0;
	result.y = 0;
	result.w = x;
	result.y = maxY;
	return result;
}