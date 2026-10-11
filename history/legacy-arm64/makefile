# Keep the original K&R function definitions in GNU C89 mode.
CC = cc
CPPFLAGS += -DHI_UX -DANSI_C -DOK_PROBLEM -DXWINDOW -DBSD
CFLAGS ?= -O2
PSY_CFLAGS = -std=gnu89 -Wno-deprecated-non-prototype -Wno-pointer-sign
LDLIBS += -lncurses

SOURCES = psy.c psybas.c psycus.c psye1.c psye2.c psye3.c psye4.c \
          psye5.c psyio.c psyreg.c psyview.c psywchar.c psywndw.c
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
	rm -f $(OBJECTS) psy
