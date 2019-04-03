#include "MapFile.h"
#include "SDL.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

MappedFile::MappedFile(const char* fileName)
{
	int f = open(fileName, O_RDONLY);
	if (f < 0) {
		SDL_SetError("open() failed");
		return;
	}

	struct stat fileMetadata;
	if (fstat(f, &fileMetadata) != 0) {
		SDL_SetError("fstat() failed");
		close(f);
		return;
	}

	size_t mappedSize = fileMetadata.st_size;

	data = static_cast<uint8_t*>(mmap(nullptr, mappedSize, PROT_READ, MAP_PRIVATE, f, 0));
	if (data == MAP_FAILED) {
		SDL_SetError("mmap() failed");
		close(f);
		return;
	}

	close(f);	// no more needed, mapping persists

	data = data;
	byteSize = mappedSize;
}

MappedFile::~MappedFile()
{
	Unmap();
}

void MappedFile::Unmap()
{
	if (!data) {
		munmap(data, byteSize);
	}
	data = nullptr;
	byteSize = 0;
}
