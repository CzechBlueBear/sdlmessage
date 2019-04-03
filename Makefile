CXX=g++ -c
CXXFLAGS=-O2 -ggdb -I /usr/include/SDL2 -I thirdparty
LINK=g++
LINKFLAGS=-lm -lSDL2 -lSDL2main -lGLEW -lGL

EXE=sdlmessage

HEADERS=MapFile.h LoadFont.h

OBJS=Main.o MapFile.o LoadFont.o

.PHONY: all clean

all: ${EXE}

clean:
	rm ${OBJS}

${EXE}: ${OBJS}
	${LINK} ${LINKFLAGS} $^ -o ${EXE}

%.o : %.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} $*.cpp -o $*.o
