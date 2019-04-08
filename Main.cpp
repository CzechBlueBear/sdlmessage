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
const int DISPLAY_NUMBER = 0;

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
	std::cerr << "    --close-on-key     Any key causes the window to close" << std::endl;
	std::cerr << "    --width <width>    Explicitly sets the window width" << std::endl;
	std::cerr << "    --height <height>  Explicitly sets the window height" << std::endl;
}

//---

class CommandLineOptions
{
public:

	bool ok = false;
	bool helpShown = false;
	bool noBorder = false;
	bool closeOnClick = false;
	bool closeOnKey = false;
	int explicitWidth = -1;
	int explicitHeight = -1;
	std::string explicitFont;

	CommandLineOptions(int argc, const char** argv);
};

//---

CommandLineOptions::CommandLineOptions(int argc, const char** argv)
{
	// indicators what value is expected after this argument
	bool follows_width = false;
	bool follows_height = false;
	bool follows_font = false;

	std::string rawMessage;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (follows_width) {
			try {
				explicitWidth = std::stoi(arg);
			}
			catch (std::invalid_argument &ex) { }
			if (explicitWidth <= 0 || explicitWidth > 16384) {
				std::cerr << "Invalid value after the --width option" << std::endl;
				return;
			}
			follows_width = false;
		}
		else if (follows_height) {
			try {
				explicitHeight = std::stoi(arg);
			} catch (std::invalid_argument &ex) { }
			if (explicitHeight <= 0 || explicitHeight > 16384) {
				std::cerr << "Invalid value after the --height option" << std::endl;
				return;
			}
			follows_height = false;
		}
		else if (follows_font) {
			explicitFont = arg;
		}
		else if (arg == "--help") {
			ShowUsage();
			helpShown = true;
			ok = true;
			return;
		}
		else if (arg == "--no-border") {
			noBorder = true;
		}
		else if (arg == "--close-on-click") {
			closeOnClick = true;
		}
		else if (arg == "--close-on-key") {
			closeOnKey = true;
		}
		else if (arg == "--width") {
			follows_width = true;
		}
		else if (arg == "--height") {
			follows_height = true;
		}
		else if (arg == "--font") {
			follows_font = true;
		}
		else {
			if (!rawMessage.empty()) {
				std::cerr << "Unrecognized argument #" << i << std::endl;
				return;
			}
			rawMessage = arg;
		}
	}
	if (rawMessage.empty()) {
		std::cerr << "No message was specified" << std::endl;
		return;
	}

	ok = true;
}

//---

int main(int argc, const char** argv)
{
	SDL::Library libSDL;
	if (!libSDL.Ok()) {
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
		return 127;
	}

	// set locale (important otherwise the default is C and we don't have Unicode!)
	std::locale::global(std::locale("en_US.UTF-8"));

	CommandLineOptions options(argc, argv);
	if (options.helpShown) return 0;
	if (!options.ok) { return 1; }

	// load the message text and convert it from multibyte to Unicode codepoints
	std::wstring messageText = MultibyteToWideString(argv[1]);

	SDL_Rect displayUsableBounds;
	SDL_GetDisplayUsableBounds(DISPLAY_NUMBER, &displayUsableBounds);

	const int windowWidth = (options.explicitWidth > 0) ? options.explicitWidth : displayUsableBounds.w/2;
	const int windowHeight = (options.explicitHeight > 0) ? options.explicitHeight : displayUsableBounds.h/8;

	SDL_Window* window = SDL_CreateWindow(
		DEFAULT_TITLE,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		windowWidth, windowHeight,
		SDL_WINDOW_ALLOW_HIGHDPI
			| (options.noBorder ? SDL_WINDOW_BORDERLESS : 0)
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

	// load the font; if no font is given explicitly, try multiple usual locations
	std::unique_ptr<MappedFile> fontFile;
	if (!options.explicitFont.empty()) {
		fontFile.reset(new MappedFile(options.explicitFont.c_str()));
	}
	else {
		for (auto candidateFile : FONT_FILE_CANDIDATES) {
			fontFile.reset(new MappedFile(candidateFile));
			if (fontFile->Ok()) break;		// candidate successful
		}
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

	SDL::Surface messageSurface(windowWidth, windowHeight, 32, SDL_PIXELFORMAT_RGBA32);
	if (!messageSurface.Ok()) {
		std::cerr << "Could not create surface: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL_Rect textRect = font.ComputeTextSize(messageText);
	int startX = windowWidth/2 - textRect.w/2;
	int startY = windowHeight/2 - textRect.h/2;

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

	SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface.GetWrapped());
	if (!messageTexture) {
		std::cerr << "Could not create message texture: " << SDL_GetError() << std::endl;
		return 127;
	}

	messageSurface.Discard();

	// the main loop: here we only need to wait for the window to be closed
	// and re-render our message if needed
	bool quitRequested = false;
	bool redrawNeeded = false;
	bool haveEvent = false;
	SDL_Event event;
	while (1) {

		// wait for any incoming events, then handle the whole batch
		SDL_WaitEvent(&event);
		do {
			if (event.type == SDL_QUIT) {	// closing button pressed
				quitRequested = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (options.closeOnKey) {	// close on *any* key?
					quitRequested = true;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					quitRequested = true;
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (options.closeOnClick) {
					quitRequested = true;
				}
			}
			else if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
					redrawNeeded = true;
				}
			}
		} while (SDL_PollEvent(&event));

		if (quitRequested) break;

		if (redrawNeeded) {
			SDL_SetRenderDrawColor(renderer, 0x0f, 0x0f, 0x0f, 0x00);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, messageTexture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
	}

	return 0;
}