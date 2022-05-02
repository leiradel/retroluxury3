#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include <png.h>
#include <jpeglib.h>

/*
########  ##    ##  ######   
##     ## ###   ## ##    ##  
##     ## ####  ## ##        
########  ## ## ## ##   #### 
##        ##  #### ##    ##  
##        ##   ### ##    ##  
##        ##    ##  ######   
*/

#define PNG_ERROR_LENGTH 256

typedef struct {
    uint8_t const* data;
    size_t size;
}
PngReader;

static void pngerror(png_structp const png, png_const_charp const error) {
    png_voidp message = png_get_error_ptr(png);
    snprintf(message, PNG_ERROR_LENGTH, "%s", error);
    longjmp(png_jmpbuf(png), 1);
}

static void pngwarn(png_structp const png, png_const_charp const error) {
    (void)png;
    (void)error;
}

static void pngread(png_structp const png, png_bytep const data, size_t const count) {
    PngReader* const reader = png_get_io_ptr(png);
    size_t const to_read = count < reader->size ? count : reader->size;
    memcpy(data, reader->data, to_read);
    reader->data += to_read;
    reader->size -= to_read;
}

static int readpng(lua_State* const L, void const* const data, size_t const size) {
    char message[PNG_ERROR_LENGTH];
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, message, pngerror, pngwarn);

    if (png == NULL) {
        return luaL_error(L, "out of memory");
    }

    png_infop info = png_create_info_struct(png);

    if (info == NULL) {
        png_destroy_read_struct(&png, NULL, NULL);
        return luaL_error(L, "out of memory");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        return luaL_error(L, message);
    }

    PngReader reader = {data, size};

    png_set_read_fn(png, &reader, pngread);
    png_read_info(png, info);

    png_uint_32 width, height;
    int bit_depth, color_type;
    png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    size_t const num_pixels = width * height;
    size_t const abgr_size = num_pixels * sizeof(uint32_t);
    uint32_t* const abgr = malloc(abgr_size);

    if (abgr == NULL) {
        png_destroy_read_struct(&png, &info, NULL);
        return luaL_error(L, "out of memory");
    }

    // Make sure we always get RGBA pixels
    if (bit_depth == 16) {
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
        png_set_scale_16(png);
#else
        png_set_strip_16(png);
#endif
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }

    // Transform transparent color to alpha
    if (png_get_valid(png, info, PNG_INFO_tRNS) != 0) {
        png_set_tRNS_to_alpha(png);
    }

    // Set alpha to opaque if non-existent
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_filler(png, 0xffff, PNG_FILLER_AFTER);
    }

    // Convert gray to RGB
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }

    // Turn on interlaced image support to read the PNG line by line
    int const num_passes = png_set_interlace_handling(png);

    png_read_update_info(png, info);

    for (int i = 0; i < num_passes; i++) {
        for (png_uint_32 y = 0; y < height; y++) {
            png_read_row(png, (uint8_t*)(abgr + y * width), NULL);
        }
    }
    
    png_read_end(png, info);
    png_destroy_read_struct(&png, &info, NULL);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    lua_pushlstring(L, (char const*)abgr, abgr_size);
    return 3;
}

/*
      ## ########  ########  ######   
      ## ##     ## ##       ##    ##  
      ## ##     ## ##       ##        
      ## ########  ######   ##   #### 
##    ## ##        ##       ##    ##  
##    ## ##        ##       ##    ##  
 ######  ##        ########  ######   
*/

typedef struct {
    struct jpeg_source_mgr pub;
    uint8_t const* data;
    size_t size;
}
JpegReader;

typedef struct {
    struct jpeg_error_mgr pub;
    char message[JMSG_LENGTH_MAX];
    jmp_buf rollback;
}
JpegError;

static void jpegdummy(j_decompress_ptr cinfo) {}

static void jpegexit(j_common_ptr cinfo) {
    JpegError* error = (JpegError*)cinfo->err;
    error->pub.output_message(cinfo);
    longjmp(error->rollback, 1);
}

static void jpegerr(j_common_ptr cinfo) {
    JpegError* error = (JpegError*)cinfo->err;
    cinfo->err->format_message(cinfo, error->message);
}

static int readjpeg(lua_State* const L, void const* const data, size_t const size) {
    struct jpeg_decompress_struct cinfo;
    memset(&cinfo, 0, sizeof(cinfo));

    JpegError error;
    memset(&error, 0, sizeof(error));

    cinfo.err = jpeg_std_error(&error.pub);
    error.pub.error_exit = jpegexit;
    error.pub.output_message = jpegerr;

    if (setjmp(error.rollback)) {
        jpeg_destroy_decompress(&cinfo);
        return 0;
    }

    JpegReader reader;
    memset(&reader, 0, sizeof(reader));

    reader.data = data;
    reader.size = size;
    reader.pub.init_source = jpegdummy;
    reader.pub.fill_input_buffer = NULL;
    reader.pub.skip_input_data = NULL;
    reader.pub.resync_to_restart = jpeg_resync_to_restart; // default
    reader.pub.term_source = jpegdummy;
    reader.pub.bytes_in_buffer = size;
    reader.pub.next_input_byte = data;

    jpeg_create_decompress(&cinfo);
    cinfo.src = (struct jpeg_source_mgr*)&reader;

    jpeg_read_header(&cinfo, TRUE);
    cinfo.out_color_space = JCS_EXT_RGBX;

    jpeg_start_decompress(&cinfo);

    JDIMENSION width = cinfo.output_width;
    JDIMENSION height = cinfo.output_height;
    size_t const num_pixels = width * height;
    size_t const abgr_size = num_pixels * sizeof(uint32_t);
    uint32_t* const abgr = malloc(abgr_size);

    if (abgr == NULL) {
        jpeg_destroy_decompress(&cinfo);
        return luaL_error(L, "out of memory");
    }

    unsigned y = 0;

    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPROW row = (uint8_t*)(abgr + y * width);
        JSAMPARRAY const array = &row;

        jpeg_read_scanlines(&cinfo, array, 1);
        y++;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    lua_pushlstring(L, (char const*)abgr, abgr_size);
    return 3;
}

/*
##       ##     ##    ###     #######  ########  ######## ##    ##         ########  ##        #######          #### ##     ##    ###     ######   ######## 
##       ##     ##   ## ##   ##     ## ##     ## ##       ###   ##         ##     ## ##       ##     ##          ##  ###   ###   ## ##   ##    ##  ##       
##       ##     ##  ##   ##  ##     ## ##     ## ##       ####  ##         ##     ## ##              ##          ##  #### ####  ##   ##  ##        ##       
##       ##     ## ##     ## ##     ## ########  ######   ## ## ##         ########  ##        #######           ##  ## ### ## ##     ## ##   #### ######   
##       ##     ## ######### ##     ## ##        ##       ##  ####         ##   ##   ##              ##          ##  ##     ## ######### ##    ##  ##       
##       ##     ## ##     ## ##     ## ##        ##       ##   ###         ##    ##  ##       ##     ##          ##  ##     ## ##     ## ##    ##  ##       
########  #######  ##     ##  #######  ##        ######## ##    ## ####### ##     ## ########  #######  ####### #### ##     ## ##     ##  ######   ######## 
*/

static int l_read(lua_State* const L) {
    size_t length = 0;
    char const* const data = luaL_checklstring(L, 1, &length);

    static uint8_t const header[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};

    if (length >= sizeof(header) && memcmp(data, header, sizeof(header)) == 0) {
        return readpng(L, data, length);
    }

    return readjpeg(L, data, length);
}

LUAMOD_API int luaopen_rl3_image(lua_State* const L) {
    static luaL_Reg const functions[] = {
        {"read", l_read},
        {NULL, NULL}
    };

    luaL_newlib(L, functions);

    return 1;
}
