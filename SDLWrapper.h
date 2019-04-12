#pragma once

// C++ wrapper for SDL.
// This is really just for fun and to make things look more object-y,
// but there is no deeper reason at all.

#include "SDL.h"
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <functional>

namespace SDL {

//---

class OkAble
{
public:

	virtual bool Ok() const = 0;
};

//---

class Library : public virtual OkAble
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
	operator SDL_Rect const*() const { return this; }
};

//---

/// Base class for objects that wrap another object using its pointer.
template<class T>
class PtrWrapper : public virtual OkAble
{
public:

	/// Returns a pointer to the wrapped object (null if the wrapper is invalid).
	T* GetWrapped() { return wrapped; }

	/// Const-variant of GetWrapped().
	const T* GetWrapped() const { return wrapped; }

	/// For automatical unwrapping when passing to a function that expects the pointer to the underlying object.
	operator T*() { return wrapped; }

	/// Const-variant of operator T*().
	operator const T*() { return wrapped; }

	/// Basic implementation of Ok(), checks whether the wrapped object exists.
	bool Ok() const { return (wrapped != nullptr); }

protected:

	/// Pointer to the wrapped object (plain pointer as it usually needs special allocation/freeing calls anyway).
	T* wrapped = nullptr;
};

//---

class Surface : public PtrWrapper<SDL_Surface>
{
public:

	/// Constructor, equivalent to SDL_CreateRGBSurfaceWithFormat().
	Surface(int width, int height, int depth, uint32_t format);

	Surface(const Surface& surface) = delete;

	/// Destructor, calls Discard().
	~Surface();

	/// Frees the wrapped object (with SDL_FreeSurface()), leaving the wrapper invalid.
	void Discard();

	SDL_PixelFormat* GetFormat() const { return wrapped ? wrapped->format : nullptr; }
	void* GetPixels() { return wrapped ? wrapped->pixels : 0; }
	int GetWidth() const { return wrapped ? wrapped->w : 0; }
	int GetHeight() const { return wrapped ? wrapped->h : 0; }
	int GetPitch() const { return wrapped ? wrapped->pitch : 0; }

	/// Blits a rectangle of pixels from this surface to the target surface.
	bool Blit(const SDL::Rect& srcRect, SDL::Surface& dest, SDL::Rect& destRect) const;
};

//---

class Texture : public PtrWrapper<SDL_Texture>
{
public:

	Texture(SDL_Renderer* renderer, Surface& src);
	~Texture();
};

//---

class Renderer : public PtrWrapper<SDL_Renderer>
{
public:

	Renderer(SDL_Window* window, int index, uint32_t flags);
	~Renderer();
};

//---

/// Wraps SDL_Timer, allows to use a C++ lambda as the payload function.
class Timer
{
public:

	enum class Type {
		kOneShot = 0,	///< Triggered only once.
		kRepeated = 1	///< Triggered repeatedly in specified intervals.
	};

	Timer(Timer::Type type, uint32_t interval, std::function<void(void)> payload);
	Timer(const Timer &src) = delete;
	~Timer();

protected:

	/// ID of the SDL timer.
	SDL_TimerID timerId = 0;

	/// SDL callback that ensures calling our payload function.
	static uint32_t CallPayload(uint32_t interval, void* indirectThis);

	/// The interval set in the constructor.
	uint32_t interval = 0;

	Timer::Type type;

	/// The payload, called when the timer elapses (probably in a different thread).
	std::function<void(void)> payload;
};

} // namespace SDL
