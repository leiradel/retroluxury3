CC ?= gcc
CFLAGS = -std=c99

DEFINES += -DMAKE_LIB # Only make the Lua libraries in onelua.c
DEFINES += -DNO_GETENV -DNO_PUTENV
DEFINES += -DPNG_NO_CONSOLE_IO -DPNG_NO_STDIO

INCLUDES += -Isrc/generated
INCLUDES += -Isrc/3rdparty/lua
INCLUDES += -Isrc/3rdparty/libjpeg-turbo
INCLUDES += -Isrc/3rdparty/libpng
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

%.lua.h: %.lua
	@echo "Creating header: $@"
	@echo "static char const `basename "$<" | sed 's/\./_/'`[] = {`cat "$<" | xxd -i`};" > "$@"

ENGINE_OBJS = \
	src/bindings/app.o \
	src/bindings/djb2.o \
	src/bindings/fetch.o \
	src/bindings/image.o \
	src/bindings/lutil.o

ENGINE_LUA = \
	src/bindings/app.lua.h \
	src/bindings/fetch.lua.h

LIBJPEG_OBJS = \
	src/3rdparty/libjpeg-turbo/jaricom.o \
	src/3rdparty/libjpeg-turbo/jcomapi.o \
	src/3rdparty/libjpeg-turbo/jdapimin.o \
	src/3rdparty/libjpeg-turbo/jdapistd.o \
	src/3rdparty/libjpeg-turbo/jdarith.o \
	src/3rdparty/libjpeg-turbo/jdcoefct.o \
	src/3rdparty/libjpeg-turbo/jdcolor.o \
	src/3rdparty/libjpeg-turbo/jddctmgr.o \
	src/3rdparty/libjpeg-turbo/jdhuff.o \
	src/3rdparty/libjpeg-turbo/jdinput.o \
	src/3rdparty/libjpeg-turbo/jdmainct.o \
	src/3rdparty/libjpeg-turbo/jdmarker.o \
	src/3rdparty/libjpeg-turbo/jdmaster.o \
	src/3rdparty/libjpeg-turbo/jdmerge.o \
	src/3rdparty/libjpeg-turbo/jdphuff.o \
	src/3rdparty/libjpeg-turbo/jdpostct.o \
	src/3rdparty/libjpeg-turbo/jdsample.o \
	src/3rdparty/libjpeg-turbo/jerror.o \
	src/3rdparty/libjpeg-turbo/jidctflt.o \
	src/3rdparty/libjpeg-turbo/jidctfst.o \
	src/3rdparty/libjpeg-turbo/jidctint.o \
	src/3rdparty/libjpeg-turbo/jidctred.o \
	src/3rdparty/libjpeg-turbo/jmemmgr.o \
	src/3rdparty/libjpeg-turbo/jmemnobs.o \
	src/3rdparty/libjpeg-turbo/jquant1.o \
	src/3rdparty/libjpeg-turbo/jquant2.o \
	src/3rdparty/libjpeg-turbo/jsimd_none.o \
	src/3rdparty/libjpeg-turbo/jutils.o

LIBPNG_OBJS = \
	src/3rdparty/libpng/pngerror.o \
	src/3rdparty/libpng/pngget.o \
	src/3rdparty/libpng/pngmem.o \
	src/3rdparty/libpng/png.o \
	src/3rdparty/libpng/pngread.o \
	src/3rdparty/libpng/pngrio.o \
	src/3rdparty/libpng/pngrtran.o \
	src/3rdparty/libpng/pngrutil.o \
	src/3rdparty/libpng/pngset.o \
	src/3rdparty/libpng/pngtrans.o \
	src/3rdparty/libpng/pngwio.o \
	src/3rdparty/libpng/pngwrite.o \
	src/3rdparty/libpng/pngwtran.o \
	src/3rdparty/libpng/pngwutil.o

LUA_OBJS = \
	src/3rdparty/lua/onelua.o

ZLIB_OBJS = \
	src/3rdparty/zlib/adler32.o \
	src/3rdparty/zlib/crc32.o \
	src/3rdparty/zlib/deflate.o \
	src/3rdparty/zlib/inffast.o \
	src/3rdparty/zlib/inflate.o \
	src/3rdparty/zlib/inftrees.o \
	src/3rdparty/zlib/trees.o \
	src/3rdparty/zlib/zutil.o

RETROLUXURY3_OBJS = $(ENGINE_OBJS)
3RDPARTY_OBJS = $(LIBJPEG_OBJS) $(LIBPNG_OBJS) $(LUA_OBJS) $(ZLIB_OBJS)

all: main

main: $(RETROLUXURY3_OBJS) $(3RDPARTY_OBJS)
	@echo "Creating executable: $@"
	@$(CC) -o "$@" $+ $(LIBS)

src/bindings/app.o: src/bindings/app.lua.h

src/bindings/fetch.o: src/bindings/fetch.lua.h

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
	@rm -f libretroluxury3.a $(RETROLUXURY3_OBJS) $(ENGINE_LUA)
	@rm -f src/generated/version.h

distclean: clean
	@echo "Cleaning up (including 3rd party libraries)"
	@rm -f $(3RDPARTY_OBJS)

.PHONY: FORCE
