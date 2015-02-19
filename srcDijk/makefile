CC = g++
DEBUG = -g
SOURCE_FILES := $(wildcard src/*.cc)
OBJ_FILES := $(addprefix obj/,$(notdir $(SOURCE_FILES:.cc=.o)))
CFLAGS = -c -Wall $(DEBUG) -I./include
LDFLAGS = -pthread

main: $(OBJ_FILES)
	$(CC) -o $@ $^

obj/%.o: src/%.cc
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f ./obj/*.o main
