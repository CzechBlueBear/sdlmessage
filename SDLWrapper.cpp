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

bool Surface::Blit(const SDL::Rect& rect, SDL::Surface& dest, SDL::Rect& destRect) const
{
	return (0 == SDL_BlitSurface(wrapped, rect, dest.GetWrapped(), destRect));
}

//---

Texture::Texture(SDL_Renderer* renderer, Surface& src)
{
	wrapped = SDL_CreateTextureFromSurface(renderer, src.GetWrapped());
}

//---

Texture::~Texture()
{
	if (wrapped) {
		SDL_DestroyTexture(wrapped);
		wrapped = nullptr;
	}
}

//---

Renderer::Renderer(SDL_Window* window, int index, uint32_t flags)
{
	wrapped = SDL_CreateRenderer(window, index, flags);
}

//---

Renderer::~Renderer()
{
	if (wrapped) {
		SDL_DestroyRenderer(wrapped);
	}
}

//---

Timer::Timer(Type type_, uint32_t interval_, std::function<void(void)> payload_)
	: type(type_), interval(interval_), payload(payload_)
{
	timerId = SDL_AddTimer(interval, CallPayload, this);
}

//---

Timer::~Timer()
{
	SDL_RemoveTimer(timerId);
}

//---

uint32_t Timer::CallPayload(uint32_t timePassed, void* indirectThis)
{
	Timer* theThis = reinterpret_cast<Timer*>(indirectThis);

	theThis->payload();

	if (theThis->type == Timer::Type::kOneShot) {

		// do not call multiple times
		return 0;
	}
	else {

		// how much we are delayed in comparison with the interval
		// (positive - we are delayed, negative - we are earlier)
		int32_t delay = timePassed - theThis->interval;

		// schedule next call, trying to compensate for delays
		return uint32_t(int32_t(theThis->interval) - delay);
	}
}

} // namespace SDL
