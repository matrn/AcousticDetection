WARNINGS+=-Wall -Wextra
CXXFLAGS+=-std=c++2a -pedantic -O3 -fomit-frame-pointer ${WARNINGS} -g -DDEBUG -DTM_DEBUG
LDLIBS+=
BUILD_DIR=build

main: main.cpp
	${CXX} -o $@ $^ ${CXXFLAGS} ${LDLIBS}

clean:
	rm -f main