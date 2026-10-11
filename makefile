# Keep the original K&R function definitions in GNU C89 mode.
CC = cc
CPPFLAGS += -DHI_UX -DANSI_C -DOK_PROBLEM -DXWINDOW -DBSD
CFLAGS ?= -O2
PSY_CFLAGS = -std=gnu89 -Wno-deprecated-non-prototype -Wno-pointer-sign
LDLIBS += -lncurses -liconv

SOURCES = psy.c psybas.c psycus.c psye1.c psye2.c psye3.c psye4.c \
          psye5.c psyio.c psyreg.c psyview.c psywchar.c psywndw.c psymem.c psyscreen.c
OBJECTS = $(SOURCES:%.c=build/%.o)

.PHONY: all clean
all: psy

psy: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

build/%.o: %.c psy.h psyext.h psyfnc.h makefile | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PSY_CFLAGS) -c $< -o $@

build:
	mkdir -p $@

clean:
	rm -f $(OBJECTS) psy build/test-memory build/psy-sanitize build/test-memory-sanitize

.PHONY: test test-sanitize
test: psy build/test-memory
	./build/test-memory
	python3 -m unittest discover -s tests -v

MEMORY_TEST_FLAGS = -DMAX_BUFF_SIZE=131072 -DMAX_BUFF_LINE=65536 -I.
SANITIZER_FLAGS = -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer

build/test-memory: tests/test_memory.c psymem.c psy.h psyext.h psyfnc.h makefile | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PSY_CFLAGS) $(MEMORY_TEST_FLAGS) tests/test_memory.c psymem.c -o $@ $(LDLIBS)

build/psy-sanitize: $(SOURCES) psy.h psyext.h psyfnc.h makefile | build
	$(CC) $(CPPFLAGS) $(PSY_CFLAGS) $(SANITIZER_FLAGS) $(SOURCES) -o $@ $(LDLIBS)

build/test-memory-sanitize: tests/test_memory.c psymem.c psy.h psyext.h psyfnc.h makefile | build
	$(CC) $(CPPFLAGS) $(PSY_CFLAGS) $(SANITIZER_FLAGS) $(MEMORY_TEST_FLAGS) tests/test_memory.c psymem.c -o $@ $(LDLIBS)

test-sanitize: build/psy-sanitize build/test-memory-sanitize
	ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ./build/test-memory-sanitize
	PSY_BINARY=build/psy-sanitize ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 python3 -m unittest discover -s tests -v
