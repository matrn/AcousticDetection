WARNINGS+=-Wall -Wextra
CXXFLAGS+=-std=c++2a -pedantic -O3 -fomit-frame-pointer ${WARNINGS} -g -DDEBUG -DTM_DEBUG
LDLIBS+=
BUILD_DIR=build

$(BUILD_DIR)/%.o: src/%.cpp src/include/*.hpp src/**/*.hpp
	@mkdir -p $(BUILD_DIR)
	# $< = first file ($^ = all files)
	${CXX} -c -o $@ $< ${CXXFLAGS} ${LDLIBS} ${OPENCVFLAGS}


main: main.cpp $(BUILD_DIR)/sampler.o
	${CXX} -o $(BUILD_DIR)/$@ $^ ${CXXFLAGS} ${LDLIBS}


corr_test: corr_test.cpp
	${CXX} -o $@ $^ ${CXXFLAGS} ${LDLIBS}
	./$@

# server: server.cpp
# 	${CXX} -o $@ $^ ${CXXFLAGS} ${LDLIBS}


clean:
	rm -f main t.csv t.raw test.wav corr_test
	rm -f ./$(BUILD_DIR)/*