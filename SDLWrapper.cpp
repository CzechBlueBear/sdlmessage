#include "SDLWrapper.h"

namespace SDL {

//---

Library::Library(uint32_t initFlags)
{
	if (SDL_Init(initFlags) == 0) {
		ok = true;
	}
}

//---

Library::~Library()
{
	SDL_Quit();
	ok = false;
}

//---

Surface::Surface(int width, int height, int depth, uint32_t format)
{
	if (width < 0 || height < 0 || depth < 0) {
		SDL_SetError("surface dimensions must be >= 0");
		return;
	}
	wrapped = SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format);
}

//---

Surface::~Surface()
{
	Discard();
}

//---

void Surface::Discard()
{
	if (wrapped) {
		SDL_FreeSurface(wrapped);
		wrapped = nullptr;
	}
}

//---

void Blit(SDL::Surface& src, SDL::Rect& srcRect, SDL::Surface& dest, SDL::Rect& destRect)
{
	SDL_BlitSurface(src.GetWrapped(), srcRect, dest.GetWrapped(), destRect);
}

} // namespace SDL
