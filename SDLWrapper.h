#pragma once

// C++ wrapper for SDL.
// This is really just for fun and to make things look more object-y,
// but there is no deeper reason at all.

#include "SDL.h"
#include <cstdint>
#include <stdexcept>
#include <optional>

namespace SDL {

//---

class Library
{
public:

	Library(uint32_t initFlags = SDL_INIT_EVERYTHING);
	Library(const Library& src) = delete;
	~Library();
	bool Ok() const { return ok; }

private:

	bool ok = false;
};

//---

class Rect : public SDL_Rect
{
public:

	/**
	 * Constructor, initializes everything to 0.
	 */
	Rect() { x = 0; y = 0; w = 0; h = 0; }

	/**
	 * Constructor, sets explicitly both position (X, Y) and width and height (W, H).
	 */
	Rect(int x_, int y_, int w_, int h_)
	{
		x = x_; y = y_; w = w_; h = h_;
	}

	/**
	 * Sets the rectangle position (X, Y) and its width and height (W, H).
	 */
	Rect &SetXYWH(int x_, int y_, int w_, int h_)
	{
		x = x_; y = y_; w = w_; h = h_;
		return *this;
	}

	operator SDL_Rect*() { return this; }
};

//---

class Surface
{
public:

	/// Constructor, equivalent to SDL_CreateRGBSurfaceWithFormat().
	Surface(int width, int height, int depth, uint32_t format);

	Surface(const Surface& surface) = delete;

	/// Destructor, calls Discard().
	~Surface();

	/// Returns true if the wrapper is valid, i.e. it has an underlying SDL object.
	bool Ok() const { return wrapped != nullptr; }

	/// Frees the wrapped object (with SDL_FreeSurface()), leaving the wrapper invalid.
	void Discard();

	/// Returns a pointer to the wrapped SDL object (null if the wrapper is invalid).
	SDL_Surface* GetWrapped() { return wrapped; }

	operator SDL_Surface*() { return wrapped; }

protected:

	SDL_Surface* wrapped = nullptr;
};

/// Copies (blits) a rectangle of pixels from the given surface into the target surface.
void Blit(SDL::Surface& src, SDL::Rect& srcRect, SDL::Surface& dest, SDL::Rect& destRect);

} // namespace SDL
