CXX=g++ -c
CXXFLAGS=-O2 -ggdb -I /usr/include/SDL2 -I thirdparty
LINK=g++
LINKFLAGS=-lm -lSDL2

EXE=sdlmessage

HEADERS=MapFile.h LoadFont.h ToUnicode.h

OBJS=Main.o MapFile.o LoadFont.o ToUnicode.o

.PHONY: all clean

all: ${EXE}

clean:
	rm ${OBJS}

${EXE}: ${OBJS}
	${LINK} ${LINKFLAGS} $^ -o ${EXE}

%.o : %.cpp ${HEADERS}
	${CXX} ${CXXFLAGS} $*.cpp -o $*.o
