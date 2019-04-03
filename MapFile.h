#pragma once
#include <cstdint>
#include <cstddef>

class MappedFile
{
public:

	/**
	 * Maps the contents of the file to memory.
	 * On error, the resulting object is invalid, and a terse error description
	 * is stored using SDL_SetError().
	 */
	MappedFile(const char* fileName);

	/**
	 * Calls Unmap().
	 */
	~MappedFile();

	/**
	 * Unmaps the file from memory, invalidating the object.
	 */
	void Unmap();

	bool Ok() const { return (data != nullptr); }
	uint8_t* GetData() { return data; }
	const uint8_t* GetData() const { return data; }
	size_t GetSize() const { return byteSize; }

protected:
	uint8_t* data = nullptr;
	size_t byteSize = 0;
};
