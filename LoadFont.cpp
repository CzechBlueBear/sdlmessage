#include "LoadFont.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <iostream>

Font::Font(const MappedFile &fontFile, float fontSize, uint32_t extraCharsetSupport)
{
	encodedCharsets = kCharsetLatin | extraCharsetSupport;

	// count the total number of characters we will need to encode,
	// to determine how large the pixel memory will need to be
	encodedCharCount = 767;	// the basic set
	if (extraCharsetSupport & kCharsetCyrillic)
		encodedCharCount += 256;
	if (extraCharsetSupport & kCharsetGreek)
		encodedCharCount += 143;

	// FIXME: This is a wild guess!
	int surfaceHeight = int(2*fontSize);
	int surfaceWidth = encodedCharCount * int(fontSize);

	fontSurface = std::make_unique<SDL::Surface>(
		surfaceWidth, surfaceHeight, 8, SDL_PIXELFORMAT_INDEX8
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

	// map character range: 0x00..0x24f, this covers:
	// - Basic Latin, aka ASCII (0x00..0x7f)
	// - Latin 1 Supplement, aka ISO-8859-1 (0x80..0xff)
	// - Latin Extended A (0x100..0x17f)
	// - Latin Extended B (0x180..0x24f)
	packedCharRanges.push_back(stbtt_pack_range {
		.font_size = fontSize,
		.first_unicode_codepoint_in_range = 0x0,
		.array_of_unicode_codepoints = nullptr,
		.num_chars = 0x24f,
		.chardata_for_range = packedCharsBasic.data(),
		.h_oversample = 0,
		.v_oversample = 0
	});

	// if requested, add Greek and Coptic (0x370..0x3ff)
	if (extraCharsetSupport & kCharsetGreek) {
		packedCharRanges.push_back(stbtt_pack_range {
			.font_size = fontSize,
			.first_unicode_codepoint_in_range = 0x370,
			.array_of_unicode_codepoints = nullptr,
			.num_chars = 0x3ff - 0x370,
			.chardata_for_range = packedCharsGreek.data(),
			.h_oversample = 0,
			.v_oversample = 0
		});
	}

	// if requested, add Cyrillic (0x400..0x4ff)
	if (extraCharsetSupport & kCharsetCyrillic) {
		packedCharRanges.push_back(stbtt_pack_range {
			.font_size = fontSize,
			.first_unicode_codepoint_in_range = 0x400,
			.array_of_unicode_codepoints = nullptr,
			.num_chars = 0x4ff - 0x400,
			.chardata_for_range = packedCharsCyrillic.data(),
			.h_oversample = 0,
			.v_oversample = 0		
		});
	}

	if (!stbtt_PackFontRanges(&packContext, fontFile.GetData(), 0, packedCharRanges.data(), packedCharRanges.size())) {
		SDL_SetError("stbtt_PackFontRanges() failed");
		stbtt_PackEnd(&packContext);
		return;
	}

	ok = true;
}

Font::~Font()
{
	ok = false;
}

const stbtt_packedchar* Font::GetPackedChar(int charCode) const
{
	if (charCode < 0x24f) {

		// this block of characters is always encoded
		return &(packedCharsBasic[charCode]);
	}
	else if ((encodedCharsets & kCharsetCyrillic) && (charCode >= 0x400 && charCode <= 0x4ff)) {
		return &(packedCharsCyrillic[charCode - 0x400]);
	}
	else if ((encodedCharsets & kCharsetGreek) && (charCode >= 0x370 && charCode <= 0x3ff)) {
		return &(packedCharsGreek[charCode - 0x370]);
	}

	// not encoded
	return nullptr;	
}

bool Font::GetGlyphRect(int charCode, SDL_Rect& result) const
{
	const stbtt_packedchar* packedChar = GetPackedChar(charCode);
	if (!packedChar) return false;

	result.x = packedChar->x0;
	result.y = packedChar->y0;
	result.w = packedChar->x1 - packedChar->x0;
	result.h = packedChar->y1 - packedChar->y0;
	return true;
}

bool Font::GetGlyphGeometry(int charCode, stbtt_packedchar &glyphGeometry) const
{
	const stbtt_packedchar* packedChar = GetPackedChar(charCode);
	if (!packedChar) return false;

	glyphGeometry = *packedChar;
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