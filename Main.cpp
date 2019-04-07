#include "MapFile.h"
#include "LoadFont.h"
#include "ToUnicode.h"
#include "SDL.h"
#include "SDLWrapper.h"
#include <memory>
#include <array>
#include <iostream>
#include <string.h>
#include <locale>
#include <sstream>

const char* DEFAULT_TITLE = "sdlmessage";
const int DEFAULT_WINDOW_WIDTH = 1024;
const int DEFAULT_WINDOW_HEIGHT = 256;

std::array<const char*, 2> FONT_FILE_CANDIDATES = {
	"/usr/share/fonts/TTF/DejaVuSans.ttf",		// Arch-ism
	"/usr/share/fonts/dejavu/DejaVuSans.ttf"	// Fedora
};

void ShowUsage()
{
	std::cerr << "sdlmessage [options] message" << std::endl << std::endl;
	std::cerr << "Shows a short, single-line message in a window and waits for the window to be closed." << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << "    --help             Shows this help text (also shown on unrecognized input)" << std::endl;
	std::cerr << "    --no-border        Show a borderless window (press Esc to dismiss it)" << std::endl;
	std::cerr << "    --close-on-click   Clicking in the window closes it" << std::endl;
}

int main(int argc, const char** argv)
{
	bool option_NoBorder = false;
	bool option_CloseOnClick = false;

	std::string rawMessage;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (arg == "--help") {
			ShowUsage();
			return 0;
		}
		if (arg == "--no-border") {
			option_NoBorder = true;
		}
		else if (arg == "--close-on-click") {
			option_CloseOnClick = true;
		}
		else {
			if (!rawMessage.empty()) {
				std::cerr << "Unrecognized argument #" << i << std::endl;
				return 1;
			}
			rawMessage = arg;
		}
	}
	if (rawMessage.empty()) {
		std::cerr << "No message was specified" << std::endl;
		ShowUsage();
		return 1;
	}

	// set locale according to environment variables
	// (important otherwise the default is C and we don't have Unicode!)
	std::locale::global(std::locale("en_US.UTF-8")); 	/* in plain C: setlocale(LC_ALL, ""); */

	// load the message text and convert it from multibyte to Unicode codepoints
	std::wstring messageText = MultibyteToWideString(argv[1]);

	if (0 != SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_INIT_EVENTS)) {
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
		atexit(SDL_Quit);
		return 127;
	}

	SDL_Window* window = SDL_CreateWindow(
		DEFAULT_TITLE,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
		SDL_WINDOW_ALLOW_HIGHDPI
			| (option_NoBorder ? SDL_WINDOW_BORDERLESS : 0)
	);
	if (!window) {
		std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
		return 127;
	}

	// load the font, trying multiple usual locations
	std::unique_ptr<MappedFile> fontFile;
	for (auto candidateFile : FONT_FILE_CANDIDATES) {
		fontFile.reset(new MappedFile(candidateFile));
		if (fontFile->Ok()) break;
	}
	if (!fontFile->Ok()) {
		std::cerr << "Could not open font file: " << SDL_GetError() << std::endl;
		return 127;
	}

	Font font(*fontFile, 32.0f);
	if (!font.Ok()) {
		std::cerr << "Could not load font: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL_Surface* messageSurface = SDL_CreateRGBSurface(0,
		DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
		32,
		0xff000000, 0x00ff0000, 0x0000ff00,
		0x000000ff	// only alpha
	);
	if (!messageSurface) {
		std::cerr << "Could not create surface: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL_Rect textRect = font.ComputeTextSize(messageText);
	int startY = DEFAULT_WINDOW_HEIGHT/2 - textRect.h/2;
	int startX = DEFAULT_WINDOW_WIDTH/2 - textRect.w/2;

	int x = startX;
	for (int i = 0; i < messageText.size(); i++) {
		stbtt_packedchar glyphGeometry;
		if (font.GetGlyphGeometry(int(messageText[i]), glyphGeometry)) {
			SDL::Rect glyphRect(
				glyphGeometry.x0,
				glyphGeometry.y0,
				glyphGeometry.x1 - glyphGeometry.x0,
				glyphGeometry.y1 - glyphGeometry.y0
			);
			SDL::Rect destRect(
				x + glyphGeometry.xoff,
				startY + glyphGeometry.yoff,
				glyphGeometry.x1 - glyphGeometry.x0,
				glyphGeometry.y1 - glyphGeometry.y0
			);
			if (0 != SDL_BlitSurface(font.GetSurface(), &glyphRect, messageSurface, &destRect)) {
				std::cerr << "Could not blit glyph: " << SDL_GetError() << std::endl;
				break;
			}
			x += glyphGeometry.xadvance;
		}
	}

	SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
	if (!messageTexture) {
		std::cerr << "Could not create message texture: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL_FreeSurface(messageSurface);

	// the main loop: here we only need to wait for the window to be closed
	// and re-render our message if needed
	bool quitRequested = false;
	SDL_Event event;
	while (1) {

		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {	// closing button pressed
			quitRequested = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				quitRequested = true;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (option_CloseOnClick) {
				quitRequested = true;
			}
		}

		if (quitRequested) break;

		SDL_SetRenderDrawColor(renderer, 0x0f, 0x0f, 0x0f, 0x00);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, messageTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}


#if 0	// this works but is incredibly ugly
	const SDL_MessageBoxButtonData buttons[1] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK" }
	};
	const SDL_MessageBoxColorScheme colorScheme = {
		{ /* .colors (.r, .g, .b) */
			/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
			{ 128, 128, 128 },
			/* [SDL_MESSAGEBOX_COLOR_TEXT] */
			{ 16, 16, 16 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
			{ 142, 142, 142 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
			{ 180, 180, 180 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
			{ 0, 0, 0 }
		}
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION,
		nullptr,
		"Message",
		messageText,
		SDL_arraysize(buttons),
		buttons,
		&colorScheme
	};

	int buttonId;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonId) < 0) {
		std::cerr << "SDL_ShowMessageBox() failed: " << SDL_GetError() << std::endl;
		return 127;
	}
#endif	// this works but is incredibly ugly

	return 0;
}