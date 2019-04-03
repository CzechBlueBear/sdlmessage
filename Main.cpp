#include "MapFile.h"
#include "LoadFont.h"
#include "SDL.h"
#include <iostream>
#include <string.h>

const char* DEFAULT_TITLE = "sdlmessage";
const int DEFAULT_WINDOW_WIDTH = 1024;
const int DEFAULT_WINDOW_HEIGHT = 256;
const char* FONT_FILE_NAME = "/usr/share/fonts/TTF/DejaVuSans.ttf";

int main(int argc, const char** argv)
{
	if (argc < 2) {
		std::cerr << "Missing argument (message to be shown)" << std::endl;
		return 1;
	}

	const char* messageText = argv[1];

	if (0 != SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_INIT_EVENTS)) {
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
		atexit(SDL_Quit);
		return 127;
	}

	SDL_Window* window = SDL_CreateWindow(
		DEFAULT_TITLE,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
		SDL_WINDOW_ALLOW_HIGHDPI);
	if (!window) {
		std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
		return 127;
	}

	MappedFile fontFile(FONT_FILE_NAME);
	if (!fontFile.Ok()) {
		std::cerr << "Could not map font file: " << FONT_FILE_NAME << ": " << SDL_GetError() << std::endl;
		return 127;
	}

	Font font(fontFile, 32.0f);
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

	for (int i = 0; i < 1024; i++) {
		static_cast<uint8_t*>(messageSurface->pixels)[i] = (i & 0xff);
	}

	SDL_BlitSurface(font.GetSurface(), SDL_Rect{0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}, messageSurface, SDL_Rect{0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT});

/*
	int x = 0;
	size_t messageLength = strlen(messageText);
	for (int i = 0; i < messageLength; i++) {
		SDL_Rect glyphRect = font.GetGlyphRect(messageText[i]);
		SDL_Rect destRect;
		destRect.x = x;
		destRect.y = 127;
		destRect.w = glyphRect.w;
		destRect.h = glyphRect.h;
		if (0 != SDL_BlitSurface(font.GetSurface(), &glyphRect, messageSurface, &destRect)) {
			std::cerr << "Could not blit glyph: " << SDL_GetError() << std::endl;
			break;
		}
		x += glyphRect.w;
		std::cout << "glyph '" << messageText[i] << "', width " << glyphRect.w << ", current pos " << x << std::endl;
	}
*/

	SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, messageSurface);
	if (!messageTexture) {
		std::cerr << "Could not create message texture: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL_FreeSurface(messageSurface);

	// the main loop: here we only need to wait for the window to be closed
	// and re-render our message if needed
	SDL_Event event;
	while (1) {
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			break;
		}
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