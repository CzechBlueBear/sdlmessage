#pragma once

// C++ wrapper for SDL.
// This is really just for fun and to make things look more object-y,
// but there is no deeper reason at all.

#include "SDL.h"
#include <cstdint>
#include <stdexcept>
#include <optional>

namespace SDL {

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
};

} // namespace SDL
