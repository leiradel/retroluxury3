CC ?= gcc
CFLAGS = -std=c99

DEFINES += -DMAKE_LIB # Only make the Lua libraries in onelua.c

INCLUDES += -Isrc/3rdparty/lua
INCLUDES += -Isrc/3rdparty/sokol
INCLUDES += -Isrc/3rdparty/sokol_gp

LIBS += -lX11 -lXi -lXcursor -lGL -ldl -lpthread -lm

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -g $(DEFINES) $(INCLUDES)
else
	CFLAGS += -O3 -DNDEBUG $(DEFINES) $(INCLUDES)
endif

%.o: %.c
	@echo "Compiling: $@"
	@$(CC) $(CFLAGS) -Wall -Werror -c "$<" -o "$@"

ENGINE_OBJS = \
	src/bindings/app.o \
	src/bindings/djb2.o

LUA_OBJS = \
	src/3rdparty/lua/onelua.o

RETROLUXURY3_OBJS = $(ENGINE_OBJS)
3RDPARTY_OBJS = $(LUA_OBJS)

all: main

main: $(RETROLUXURY3_OBJS) $(3RDPARTY_OBJS)
	@echo "Creating executable: $@"
	@$(CC) -o "$@" $+ $(LIBS)

src/3rdparty/lua/onelua.o: src/3rdparty/lua/onelua.c
	@echo "Compiling: $@"
	@$(CC) $(CFLAGS) -c $< -o $@

src/generated/version.h: FORCE
	@echo "Creating version header: $@"
	@cat etc/version.templ.h \
		| sed s/\&HASH/`git rev-parse HEAD | tr -d "\n"`/g \
		| sed s/\&VERSION/`git tag | sort -r -V | head -n1 | tr -d "\n"`/g \
		| sed s/\&DATE/`date -Iseconds`/g \
		> $@

clean: FORCE
	@echo "Cleaning up"
	@rm -f libretroluxury3.a $(RETROLUXURY3_OBJS)
	@rm -f src/generated/version.h

distclean: clean
	@echo "Cleaning up (including 3rd party libraries)"
	@rm -f $(3RDPARTY_OBJS)

.PHONY: FORCE
