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

//---

void ShowUsage()
{
	std::cerr << "Usage:" << std::endl;
	std::cerr << "    sdlmessage [options] message" << std::endl << std::endl;
	std::cerr << "Shows a short, single-line message in a window and waits for the window to be closed." << std::endl << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << "    --help             Shows this help text (also shown on unrecognized input)" << std::endl;
	std::cerr << "    --x                X coordinate of the window" << std::endl;
	std::cerr << "    --y                Y coordinate of the window" << std::endl;
	std::cerr << "    --no-border        Show a borderless window (press Esc to dismiss it)" << std::endl;
	std::cerr << "    --width <width>    Explicitly sets the window width" << std::endl;
	std::cerr << "    --height <height>  Explicitly sets the window height" << std::endl;
	std::cerr << "    --font <path>      Complete path to font to use" << std::endl;
	std::cerr << "    --close-on-click   Clicking in the window closes it" << std::endl;
	std::cerr << "    --close-on-key     Any key causes the window to close" << std::endl;
	std::cerr << "    --close-after      Close automatically after given number of milliseconds" << std::endl;
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
	int windowX = -1;
	int windowY = -1;
	int32_t closingDelay = -1;
	std::string explicitFont;

	CommandLineOptions(int argc, const char** argv);
};

//---

enum class ValueExpected {
	kNone = 0,

	// numeric values
	kWindowX = 1,
	kWindowY,
	kWindowWidth,
	kWindowHeight,
	kClosingDelay,

	// string values
	kFont = 100
};

//---

CommandLineOptions::CommandLineOptions(int argc, const char** argv)
{
	// what value is expected after this argument
	auto expected = ValueExpected::kNone;

	std::string rawMessage;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (expected != ValueExpected::kNone) {
			if (expected == ValueExpected::kFont) {
				explicitFont = arg;
			}
			else {
				try {
					int value = std::stoi(arg);
					switch (expected) {
						case ValueExpected::kWindowX:
							windowX = value;
							break;
						case ValueExpected::kWindowY:
							windowY = value;
							break;
						case ValueExpected::kWindowWidth:
							if (value <= 0 || value > 16384) {
								std::cerr << "error: window width out of bounds" << std::endl;
								return;
							}
							explicitWidth = value;
							break;
						case ValueExpected::kWindowHeight:
							explicitHeight = value;
							if (value <= 0 || value > 16384) {
								std::cerr << "error: window height out of bounds" << std::endl;
								return;
							}
							break;
						case ValueExpected::kClosingDelay:
							closingDelay = value;
							break;
					}
				}
				catch (std::invalid_argument &ex) {
					std::cerr << "error: invalid numeric value as command-line argument #" << i << std::endl;
					return;
				}
			}
			expected = ValueExpected::kNone;
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
		else if (arg == "--close-after") {
			expected = ValueExpected::kClosingDelay;
		}
		else if (arg == "--x") {
			expected = ValueExpected::kWindowX;
		}
		else if (arg == "--y") {
			expected = ValueExpected::kWindowY;
		}
		else if (arg == "--width") {
			expected = ValueExpected::kWindowWidth;
		}
		else if (arg == "--height") {
			expected = ValueExpected::kWindowHeight;
		}
		else if (arg == "--font") {
			expected = ValueExpected::kFont;
		}
		else {
			if (!rawMessage.empty()) {
				std::cerr << "error: unrecognized argument #" << i << std::endl;
				return;
			}
			rawMessage = arg;
		}
	}
	if (rawMessage.empty()) {
		std::cerr << "error: no message was specified" << std::endl;
		return;
	}

	ok = true;
}

//---

int main(int argc, const char** argv)
{
	SDL::Library libSDL;
	if (!libSDL.Ok()) {
		std::cerr << "error: could not initialize SDL: " << SDL_GetError() << std::endl;
		return 127;
	}

	// set locale (important otherwise the default is C and we don't have Unicode!)
	std::locale::global(std::locale("en_US.UTF-8"));

	CommandLineOptions options(argc, argv);
	if (options.helpShown) return 0;
	if (!options.ok) { ShowUsage(); return 1; }

	// load the message text and convert it from multibyte to Unicode codepoints
	std::wstring messageText = MultibyteToWideString(argv[1]);

	SDL_Rect displayUsableBounds;
	SDL_GetDisplayUsableBounds(DISPLAY_NUMBER, &displayUsableBounds);

	const int windowWidth = (options.explicitWidth > 0) ? options.explicitWidth : displayUsableBounds.w/2;
	const int windowHeight = (options.explicitHeight > 0) ? options.explicitHeight : displayUsableBounds.h/8;

	SDL_Window* window = SDL_CreateWindow(
		DEFAULT_TITLE,
		(options.windowX >= 0 ? options.windowX : SDL_WINDOWPOS_CENTERED),
		(options.windowY >= 0 ? options.windowY : SDL_WINDOWPOS_CENTERED),
		windowWidth, windowHeight,
		SDL_WINDOW_ALLOW_HIGHDPI
			| (options.noBorder ? SDL_WINDOW_BORDERLESS : 0)
	);
	if (!window) {
		std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL::Renderer renderer(window, -1, 0);
	if (!renderer.Ok()) {
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

	Font font(*fontFile, 32.0f, Font::kCharsetCyrillic|Font::kCharsetGreek);
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
			if (!font.GetSurface().Blit(glyphRect, messageSurface, destRect)) {
				std::cerr << "Could not blit glyph: " << SDL_GetError() << std::endl;
				break;
			}
			x += glyphGeometry.xadvance;
		}
	}

	SDL::Texture messageTexture(renderer, messageSurface);
	if (!messageTexture.Ok()) {
		std::cerr << "Could not create message texture: " << SDL_GetError() << std::endl;
		return 127;
	}

	messageSurface.Discard();

	SDL::EventLoop eventLoop(libSDL);

	// install timer for closing after specified time; it sends a UserEvent we then catch in the event loop
	std::unique_ptr<SDL::Timer> closingTimer;
	if (options.closingDelay > 0) {
		closingTimer.reset(new SDL::Timer(SDL::Timer::Type::kOneShot, options.closingDelay, [&eventLoop]{
			eventLoop.PushUserEvent(0);
		}));
	};

	eventLoop.OnRedraw = [&renderer, &messageTexture](){
		SDL_SetRenderDrawColor(renderer, 0x0f, 0x0f, 0x0f, 0x00);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, messageTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
	};
	eventLoop.OnKey = [&eventLoop, options](const SDL_KeyboardEvent &event) {
		if (options.closeOnKey) {	// close on *any* key?
			eventLoop.quitRequested = true;
		}
		if (event.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			eventLoop.quitRequested = true;
		}
	};
	eventLoop.OnMouseButton = [&eventLoop, options](const SDL_MouseButtonEvent &event) {
		if (options.closeOnClick) {
			eventLoop.quitRequested = true;
		}
	};
	eventLoop.OnUserEvent = [&eventLoop](const SDL_UserEvent& event) {

		// quit when we receive the user event sent by the timer
		eventLoop.quitRequested = true;
	};
	eventLoop.Run();

	return 0;
}