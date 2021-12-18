ROOT_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
SRC=$(wildcard src/*.cc)
TESTS=$(wildcard src/*.test.cpp) $(filter-out src/api.cc, $(SRC))
TEST_BIN=build/test/test
LIBS= -lpistache -lfmt -lglpk -lz

TEST_LIBS=-lCatch2WithMain
BIN=build/bin/lp++

CC=clang++
CFLAGS=-Wall -std=c++17 $(INCLUDE) $(LIBS) -I$(ROOT_DIR)/include/

build-debug: clean
	$(CC) $(SRC) -g -o $(BIN) $(CFLAGS) 

debug: build-debug
	gdb $(BIN)

bin: $(SRC)
	$(CC) $(SRC) -o $(BIN) $(CFLAGS)

run: bin
	chmod +x $(BIN)
	$(BIN)

build-tests: 
	$(CC) $(TESTS) $(CFLAGS) $(TEST_LIBS) -O3 -o $(TEST_BIN) 

test: build-tests
	chmod +x $(TEST_BIN)
	$(TEST_BIN)

build-test-debug: $(TESTS)
	$(CC) $(TESTS) $(CFLAGS) $(TEST_LIBS) -g -o $(TEST_BIN)

test-debug: build-test-debug
	gdb $(TEST_BIN)

clean:
	rm -rf $(BIN) $(TEST_BIN)

watch:
	while true; do make $(WATCHMAKE) & inotifywait -qre close_write src | pkill lp++; done;