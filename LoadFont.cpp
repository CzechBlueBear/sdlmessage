#include "LoadFont.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <iostream>

Font::Font(const MappedFile &fontFile, float fontSize)
{
	fontSurface = SDL_CreateRGBSurface(
		0,
		DEFAULT_FONT_SURFACE_WIDTH, DEFAULT_FONT_SURFACE_HEIGHT,
		8, 0, 0, 0, 0
	);
	if (!fontSurface) {
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
	SDL_SetPaletteColors(fontSurface->format->palette, colorRamp, 0, 256);

	if (!stbtt_InitFont(&fontInfo, fontFile.GetData(), 0)) { /*stbtt_GetFontOffsetForIndex(fontFile.GetData(), 0) */
		SDL_SetError("stbtt_InitFont() failed");
		return;
	}

	stbtt_pack_context packContext = { 0 };
	if (!stbtt_PackBegin(
		&packContext,
		static_cast<uint8_t*>(fontSurface->pixels),
		fontSurface->w, fontSurface->h, fontSurface->pitch,
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
		return;
	}

	for (int i = 32; i < NUMBER_OF_CHARS; i++) {
		std::cout << "glyph #" << i << "'" << char(i)
			<< "' placement: [" << packedChars[i].x0 << " " << packedChars[i].y0
			<< " " << packedChars[i].x1 << " " << packedChars[i].y1 << "]" << std::endl;
	}

	stbtt_PackEnd(&packContext);

	ok = true;
}

Font::~Font()
{
	ok = false;
	if (fontSurface) {
		SDL_FreeSurface(fontSurface);
		fontSurface = nullptr;
	}
}

SDL_Rect Font::GetGlyphRect(int charCode) const
{
	SDL_Rect result = { 0 };
	if (charCode > NUMBER_OF_CHARS) return result;

	const stbtt_packedchar& packedChar = packedChars[charCode];
	result.x = packedChar.x0;
	result.y = packedChar.y0;
	result.w = packedChar.x1 - packedChar.x0;
	result.y = packedChar.y1 - packedChar.y0;
	return result;
}
