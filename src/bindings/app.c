#define _POSIX_C_SOURCE 199309L
#include <time.h> // for clock_gettime

#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include <sokol_app.h>

#include "djb2.h"

/*
 ######     ###    ########  ########          ######## ##     ## ######## ##    ## ######## 
##    ##   ## ##   ##     ## ##     ##         ##       ##     ## ##       ###   ##    ##    
##        ##   ##  ##     ## ##     ##         ##       ##     ## ##       ####  ##    ##    
 ######  ##     ## ########  ########          ######   ##     ## ######   ## ## ##    ##    
      ## ######### ##        ##                ##        ##   ##  ##       ##  ####    ##    
##    ## ##     ## ##        ##                ##         ## ##   ##       ##   ###    ##    
 ######  ##     ## ##        ##        ####### ########    ###    ######## ##    ##    ##    
*/

#define SAPP_EVENT_MT "sapp_event"

typedef struct Event {
    sapp_event event;
    struct Event* next;
    int my_ref;
}
Event;

static Event* free_events = NULL;

static Event* event_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SAPP_EVENT_MT);
}

static int event_touches_iterator(lua_State* const L) {
    Event const* const self = event_check(L, 1);
    lua_Integer const index = luaL_checkinteger(L, 2);

    if (index < self->event.num_touches) {
        lua_pushinteger(L, index + 1);
        lua_pushinteger(L, self->event.touches[index].identifier);
        lua_pushnumber(L, self->event.touches[index].pos_x);
        lua_pushnumber(L, self->event.touches[index].pos_y);
        lua_pushboolean(L, self->event.touches[index].changed);
        return 5;
    }

    lua_pushnil(L);
    return 1;
}

static int event_index(lua_State* const L) {
    Event const* const self = event_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0xd9cf79d8): /* frame_count */
            lua_pushinteger(L, self->event.frame_count);
            return 1;

        case DJB2HASH_C(0x7c9ebd07): /* type */
            lua_pushinteger(L, self->event.type);
            return 1;

        case DJB2HASH_C(0xbb20b728): /* key_code */
            lua_pushinteger(L, self->event.key_code);
            return 1;

        case DJB2HASH_C(0x34a71d1d): /* char_code */
            lua_pushinteger(L, self->event.char_code);
            return 1;

        case DJB2HASH_C(0x287a2f0e): /* key_repeat */
            lua_pushboolean(L, self->event.key_repeat);
            return 1;

        case DJB2HASH_C(0x09c26f07): /* modifiers */
            lua_pushinteger(L, self->event.modifiers);
            return 1;

        case DJB2HASH_C(0xe42516a9): /* mouse_button */
            lua_pushinteger(L, self->event.mouse_button);
            return 1;

        case DJB2HASH_C(0xd5be1645): /* mouse_x */
            lua_pushnumber(L, self->event.mouse_x);
            return 1;

        case DJB2HASH_C(0xd5be1646): /* mouse_y */
            lua_pushnumber(L, self->event.mouse_y);
            return 1;

        case DJB2HASH_C(0x8d80dcc9): /* mouse_dx */
            lua_pushnumber(L, self->event.mouse_dx);
            return 1;

        case DJB2HASH_C(0x8d80dcca): /* mouse_dy */
            lua_pushnumber(L, self->event.mouse_dy);
            return 1;

        case DJB2HASH_C(0x740d326b): /* scroll_x */
            lua_pushnumber(L, self->event.scroll_x);
            return 1;

        case DJB2HASH_C(0x740d326c): /* scroll_y */
            lua_pushnumber(L, self->event.scroll_y);
            return 1;

        case DJB2HASH_C(0xf08d0700): /* touches */
            lua_pushcfunction(L, event_touches_iterator);
            lua_pushvalue(L, 1);
            lua_pushinteger(L, 0);
            return 3;

        case DJB2HASH_C(0x290c545c): /* window_width */
            lua_pushinteger(L, self->event.window_width);
            return 1;

        case DJB2HASH_C(0x2753a375): /* window_height */
            lua_pushinteger(L, self->event.window_height);
            return 1;

        case DJB2HASH_C(0xec986829): /* framebuffer_width */
            lua_pushinteger(L, self->event.framebuffer_width);
            return 1;

        case DJB2HASH_C(0x5c6230e2): /* framebuffer_height */
            lua_pushinteger(L, self->event.framebuffer_height);
            return 1;
    }

    return luaL_error(L, "unknown field %s in %s", key, SAPP_EVENT_MT);
}

#ifndef NDEBUG
static int event_newindex(lua_State* const L) {
    event_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0xd9cf79d8): /* frame_count */
        case DJB2HASH_C(0x7c9ebd07): /* type */
        case DJB2HASH_C(0xbb20b728): /* key_code */
        case DJB2HASH_C(0x34a71d1d): /* char_code */
        case DJB2HASH_C(0x287a2f0e): /* key_repeat */
        case DJB2HASH_C(0x09c26f07): /* modifiers */
        case DJB2HASH_C(0xe42516a9): /* mouse_button */
        case DJB2HASH_C(0xd5be1645): /* mouse_x */
        case DJB2HASH_C(0xd5be1646): /* mouse_y */
        case DJB2HASH_C(0x8d80dcc9): /* mouse_dx */
        case DJB2HASH_C(0x8d80dcca): /* mouse_dy */
        case DJB2HASH_C(0x740d326b): /* scroll_x */
        case DJB2HASH_C(0x740d326c): /* scroll_y */
        case DJB2HASH_C(0xf08d0700): /* touches */
        case DJB2HASH_C(0x290c545c): /* window_width */
        case DJB2HASH_C(0x2753a375): /* window_height */
        case DJB2HASH_C(0xec986829): /* framebuffer_width */
        case DJB2HASH_C(0x5c6230e2): /* framebuffer_height */
            return luaL_error(L, "cannot change field %s in %s, object is read-only", key, SAPP_EVENT_MT);
    }

    return luaL_error(L, "unknown field %s in %s", key, SAPP_EVENT_MT);
}
#endif

static int event_gc(lua_State* const L) {
    Event* const self = event_check(L, 1);

    self->next = free_events;
    free_events = self;

    lua_pushvalue(L, 1);
    self->my_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

static Event* event_push(lua_State* const L) {
    Event* self = NULL;

    if (free_events != NULL) {
        self = free_events;
        free_events = self->next;

        lua_rawgeti(L, LUA_REGISTRYINDEX, self->my_ref);

        luaL_unref(L, LUA_REGISTRYINDEX, self->my_ref);
        self->my_ref = LUA_NOREF;
    }
    else {
        self = lua_newuserdata(L, sizeof(*self));
        self->next = NULL;
        self->my_ref = LUA_NOREF;

        if (luaL_newmetatable(L, SAPP_EVENT_MT) != 0) {
            lua_pushcfunction(L, event_index);
            lua_setfield(L, -2, "__index");

#ifndef NDEBUG
            lua_pushcfunction(L, event_newindex);
            lua_setfield(L, -2, "__newindex");
#endif

            lua_pushcfunction(L, event_gc);
            lua_setfield(L, -2, "__gc");
        }

        lua_setmetatable(L, -2);
    }

    return self;
}

/*
 ######     ###    ########  ########          ########  ########  ######   ######  
##    ##   ## ##   ##     ## ##     ##         ##     ## ##       ##    ## ##    ## 
##        ##   ##  ##     ## ##     ##         ##     ## ##       ##       ##       
 ######  ##     ## ########  ########          ##     ## ######    ######  ##       
      ## ######### ##        ##                ##     ## ##             ## ##       
##    ## ##     ## ##        ##                ##     ## ##       ##    ## ##    ## 
 ######  ##     ## ##        ##        ####### ########  ########  ######   ######  
*/

#define SAPP_DESC_MT "sapp_desc"

static int init_cb_ref = LUA_NOREF;
static int frame_cb_ref = LUA_NOREF;
static int cleanup_cb_ref = LUA_NOREF;
static int event_cb_ref = LUA_NOREF;
static int fail_cb_ref = LUA_NOREF;

static char* window_title = NULL;
static char* html5_canvas_name = NULL;

typedef struct {
    sapp_desc desc;
}
Desc;

static Desc* desc_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SAPP_DESC_MT);
}

static int desc_index(lua_State* const L) {
    Desc const* const self = desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x9ea747dd): /* init_cb */
            lua_rawgeti(L, LUA_REGISTRYINDEX, init_cb_ref);
            return 1;

        case DJB2HASH_C(0xcf8f0114): /* frame_cb */
            lua_rawgeti(L, LUA_REGISTRYINDEX, frame_cb_ref);
            return 1;

        case DJB2HASH_C(0x54296131): /* cleanup_cb */
            lua_rawgeti(L, LUA_REGISTRYINDEX, cleanup_cb_ref);
            return 1;

        case DJB2HASH_C(0x20a513eb): /* event_cb */
            lua_rawgeti(L, LUA_REGISTRYINDEX, event_cb_ref);
            return 1;

        case DJB2HASH_C(0x99611e65): /* fail_cb */
            lua_rawgeti(L, LUA_REGISTRYINDEX, fail_cb_ref);
            return 1;

        case DJB2HASH_C(0x10a3b0a5): /* width */
            lua_pushinteger(L, self->desc.width);
            return 1;

        case DJB2HASH_C(0x01d688de): /* height */
            lua_pushinteger(L, self->desc.height);
            return 1;

        case DJB2HASH_C(0x7c5fd34f): /* sample_count */
            lua_pushinteger(L, self->desc.sample_count);
            return 1;

        case DJB2HASH_C(0x6d3c22c4): /* swap_interval */
            lua_pushinteger(L, self->desc.swap_interval);
            return 1;

        case DJB2HASH_C(0x00e7ce01): /* high_dpi */
            lua_pushboolean(L, self->desc.high_dpi);
            return 1;

        case DJB2HASH_C(0x479bc0b8): /* fullscreen */
            lua_pushboolean(L, self->desc.fullscreen);
            return 1;

        case DJB2HASH_C(0x0f176c2b): /* alpha */
            lua_pushboolean(L, self->desc.alpha);
            return 1;

        case DJB2HASH_C(0x28d64dde): /* window_title */
            lua_pushstring(L, self->desc.window_title);
            return 1;

        case DJB2HASH_C(0x5a422201): /* user_cursor */
            lua_pushboolean(L, self->desc.user_cursor);
            return 1;

        case DJB2HASH_C(0xa84cf4fb): /* enable_clipboard */
            lua_pushboolean(L, self->desc.enable_clipboard);
            return 1;

        case DJB2HASH_C(0xa951f90f): /* clipboard_size */
            lua_pushinteger(L, self->desc.clipboard_size);
            return 1;

        case DJB2HASH_C(0x2a57756c): /* enable_dragndrop */
            lua_pushboolean(L, self->desc.enable_clipboard);
            return 1;

        case DJB2HASH_C(0xacfb972a): /* max_dropped_files */
            lua_pushinteger(L, self->desc.max_dropped_files);
            return 1;

        case DJB2HASH_C(0xb2f6a8c4): /* max_dropped_file_path_length */
            lua_pushinteger(L, self->desc.max_dropped_file_path_length);
            return 1;

        case DJB2HASH_C(0x7c98572e): /* icon */
            lua_pushnil(L); // TODO
            return 1;

        case DJB2HASH_C(0xede27b02): /* gl_force_gles2 */
            lua_pushboolean(L, self->desc.gl_force_gles2);
            return 1;

        case DJB2HASH_C(0xc19fd550): /* win32_console_utf8 */
            lua_pushinteger(L, self->desc.width);
            return 1;

        case DJB2HASH_C(0x7ec98e5d): /* win32_console_create */
            lua_pushboolean(L, self->desc.win32_console_create);
            return 1;

        case DJB2HASH_C(0x7a4ba59e): /* win32_console_attach */
            lua_pushboolean(L, self->desc.win32_console_attach);
            return 1;

        case DJB2HASH_C(0x120621aa): /* html5_canvas_name */
            lua_pushstring(L, self->desc.html5_canvas_name);
            return 1;

        case DJB2HASH_C(0xb5b5a41b): /* html5_canvas_resize */
            lua_pushboolean(L, self->desc.html5_canvas_resize);
            return 1;

        case DJB2HASH_C(0xdc34fd9e): /* html5_preserve_drawing_buffer */
            lua_pushboolean(L, self->desc.html5_preserve_drawing_buffer);
            return 1;

        case DJB2HASH_C(0xcbe15833): /* html5_premultiplied_alpha */
            lua_pushboolean(L, self->desc.html5_premultiplied_alpha);
            return 1;

        case DJB2HASH_C(0xd517f52d): /* html5_ask_leave_site */
            lua_pushboolean(L, self->desc.html5_ask_leave_site);
            return 1;

        case DJB2HASH_C(0xcc35e51f): /* ios_keyboard_resizes_canvas */
            lua_pushboolean(L, self->desc.ios_keyboard_resizes_canvas);
            return 1;
    }

    return luaL_error(L, "unknown field %s in %s", key, SAPP_DESC_MT);
}

#ifndef NDEBUG
static int desc_newindex(lua_State* const L) {
    desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x9ea747dd): /* init_cb */
        case DJB2HASH_C(0xcf8f0114): /* frame_cb */
        case DJB2HASH_C(0x54296131): /* cleanup_cb */
        case DJB2HASH_C(0x20a513eb): /* event_cb */
        case DJB2HASH_C(0x99611e65): /* fail_cb */
        case DJB2HASH_C(0x10a3b0a5): /* width */
        case DJB2HASH_C(0x01d688de): /* height */
        case DJB2HASH_C(0x7c5fd34f): /* sample_count */
        case DJB2HASH_C(0x6d3c22c4): /* swap_interval */
        case DJB2HASH_C(0x00e7ce01): /* high_dpi */
        case DJB2HASH_C(0x479bc0b8): /* fullscreen */
        case DJB2HASH_C(0x0f176c2b): /* alpha */
        case DJB2HASH_C(0x28d64dde): /* window_title */
        case DJB2HASH_C(0x5a422201): /* user_cursor */
        case DJB2HASH_C(0xa84cf4fb): /* enable_clipboard */
        case DJB2HASH_C(0xa951f90f): /* clipboard_size */
        case DJB2HASH_C(0x2a57756c): /* enable_dragndrop */
        case DJB2HASH_C(0xacfb972a): /* max_dropped_files */
        case DJB2HASH_C(0xb2f6a8c4): /* max_dropped_file_path_length */
        case DJB2HASH_C(0x7c98572e): /* icon */
        case DJB2HASH_C(0xede27b02): /* gl_force_gles2 */
        case DJB2HASH_C(0xc19fd550): /* win32_console_utf8 */
        case DJB2HASH_C(0x7ec98e5d): /* win32_console_create */
        case DJB2HASH_C(0x7a4ba59e): /* win32_console_attach */
        case DJB2HASH_C(0x120621aa): /* html5_canvas_name */
        case DJB2HASH_C(0xb5b5a41b): /* html5_canvas_resize */
        case DJB2HASH_C(0xdc34fd9e): /* html5_preserve_drawing_buffer */
        case DJB2HASH_C(0xcbe15833): /* html5_premultiplied_alpha */
        case DJB2HASH_C(0xd517f52d): /* html5_ask_leave_site */
        case DJB2HASH_C(0xcc35e51f): /* ios_keyboard_resizes_canvas */
            return luaL_error(L, "cannot change field %s in %s, object is read-only", key, SAPP_DESC_MT);
    }

    return luaL_error(L, "unknown field %s in %s", key, SAPP_DESC_MT);
}
#endif

static Desc* desc_push(lua_State* const L) {
    Desc* const self = lua_newuserdata(L, sizeof(*self));

    if (luaL_newmetatable(L, SAPP_DESC_MT) != 0) {
        lua_pushcfunction(L, desc_index);
        lua_setfield(L, -2, "__index");

#ifndef NDEBUG
        lua_pushcfunction(L, desc_newindex);
        lua_setfield(L, -2, "__newindex");
#endif
    }

    lua_setmetatable(L, -2);
    return self;
}

/*
 ######   #######  ##    ##  #######  ##                  ###    ########  ########  
##    ## ##     ## ##   ##  ##     ## ##                 ## ##   ##     ## ##     ## 
##       ##     ## ##  ##   ##     ## ##                ##   ##  ##     ## ##     ## 
 ######  ##     ## #####    ##     ## ##               ##     ## ########  ########  
      ## ##     ## ##  ##   ##     ## ##               ######### ##        ##        
##    ## ##     ## ##   ##  ##     ## ##               ##     ## ##        ##        
 ######   #######  ##    ##  #######  ######## ####### ##     ## ##        ##        
*/

static int l_isvalid(lua_State* L) {
    bool const isvalid = sapp_isvalid();
    lua_pushboolean(L, isvalid);
    return 1;
}

static int l_width(lua_State* L) {
    int const width = sapp_width();
    lua_pushinteger(L, width);
    return 1;
}

static int l_height(lua_State* L) {
    int const height = sapp_height();
    lua_pushinteger(L, height);
    return 1;
}

static int l_color_format(lua_State* L) {
    int const color_format = sapp_color_format();
    lua_pushinteger(L, color_format);
    return 1;
}

static int l_depth_format(lua_State* L) {
    int const depth_format = sapp_depth_format();
    lua_pushinteger(L, depth_format);
    return 1;
}

static int l_sample_count(lua_State* L) {
    int const sample_count = sapp_sample_count();
    lua_pushinteger(L, sample_count);
    return 1;
}

static int l_high_dpi(lua_State* L) {
    bool const high_dpi = sapp_high_dpi();
    lua_pushboolean(L, high_dpi);
    return 1;
}

static int l_dpi_scale(lua_State* L) {
    float const dpi_scale = sapp_dpi_scale();
    lua_pushnumber(L, dpi_scale);
    return 1;
}

static int l_show_keyboard(lua_State* L) {
    bool const show = lua_toboolean(L, 1) != 0;
    sapp_show_keyboard(show);
    return 0;
}

static int l_keyboard_shown(lua_State* L) {
    bool const keyboard_shown = sapp_keyboard_shown();
    lua_pushboolean(L, keyboard_shown);
    return 1;
}

static int l_is_fullscreen(lua_State* L) {
    bool const is_fullscreen = sapp_is_fullscreen();
    lua_pushboolean(L, is_fullscreen);
    return 1;
}

static int l_toggle_fullscreen(lua_State* L) {
    (void)L;
    sapp_toggle_fullscreen();
    return 0;
}

static int l_show_mouse(lua_State* L) {
    bool const show = lua_toboolean(L, 1) != 0;
    sapp_show_mouse(show);
    return 0;
}

static int l_mouse_shown(lua_State* L) {
    bool const mouse_shown = sapp_mouse_shown();
    lua_pushboolean(L, mouse_shown);
    return 1;
}

static int l_lock_mouse(lua_State* L) {
    bool const lock = lua_toboolean(L, 1) != 0;
    sapp_lock_mouse(lock);
    return 0;
}

static int l_mouse_locked(lua_State* L) {
    bool const mouse_locked = sapp_mouse_locked();
    lua_pushboolean(L, mouse_locked);
    return 1;
}

static int l_query_desc(lua_State* L) {
    Desc* const self = desc_push(L);
    self->desc = sapp_query_desc();
    return 1;
}

static int l_request_quit(lua_State* L) {
    (void)L;
    sapp_request_quit();
    return 0;
}

static int l_cancel_quit(lua_State* L) {
    (void)L;
    sapp_cancel_quit();
    return 0;
}

static int l_quit(lua_State* L) {
    (void)L;
    sapp_quit();
    return 0;
}

static int l_consume_event(lua_State* L) {
    (void)L;
    sapp_consume_event();
    return 0;
}

static int l_frame_count(lua_State* L) {
    uint64_t const frame_count = sapp_frame_count();
    lua_pushinteger(L, frame_count);
    return 1;
}

static int l_frame_duration(lua_State* L) {
    double const frame_duration = sapp_frame_duration();
    lua_pushnumber(L, frame_duration);
    return 1;
}

static int l_set_clipboard_string(lua_State* L) {
    char const* const string = luaL_checkstring(L, 1);
    sapp_set_clipboard_string(string);
    return 0;
}

static int l_get_clipboard_string(lua_State* L) {
    char const* const clipboard_string = sapp_get_clipboard_string();
    lua_pushstring(L, clipboard_string);
    return 1;
}

static int l_set_window_title(lua_State* L) {
    char const* const string = luaL_checkstring(L, 1);
    sapp_set_window_title(string);
    return 0;
}

static int l_set_icon(lua_State* L) {
    return luaL_error(L, "sapp_set_icon not implemented");
}

static int l_get_num_dropped_files(lua_State* L) {
    int const num_dropped_files = sapp_get_num_dropped_files();
    lua_pushinteger(L, num_dropped_files);
    return 1;
}

static int l_get_dropped_file_path(lua_State* L) {
    lua_Integer const index = luaL_checkinteger(L, 1);
    char const* const dropped_file_path = sapp_get_dropped_file_path(index);
    lua_pushstring(L, dropped_file_path);
    return 1;
}

static int l_gles2(lua_State* L) {
    lua_pushboolean(L, sapp_gles2());
    return 1;
}

static int l_html5_ask_leave_site(lua_State* L) {
    bool const ask = lua_toboolean(L, 1);
    sapp_html5_ask_leave_site(ask);
    return 0;
}

static int l_html5_get_dropped_file_size(lua_State* L) {
    lua_Integer const index = luaL_checkinteger(L, 1);
    uint32_t const dropped_file_size = sapp_html5_get_dropped_file_size(index);
    lua_pushinteger(L, dropped_file_size);
    return 1;
}

/*
##       ##     ##    ###     #######  ########  ######## ##    ##          ######   #######  ##    ##  #######  ##                  ###    ########  ########  
##       ##     ##   ## ##   ##     ## ##     ## ##       ###   ##         ##    ## ##     ## ##   ##  ##     ## ##                 ## ##   ##     ## ##     ## 
##       ##     ##  ##   ##  ##     ## ##     ## ##       ####  ##         ##       ##     ## ##  ##   ##     ## ##                ##   ##  ##     ## ##     ## 
##       ##     ## ##     ## ##     ## ########  ######   ## ## ##          ######  ##     ## #####    ##     ## ##               ##     ## ########  ########  
##       ##     ## ######### ##     ## ##        ##       ##  ####               ## ##     ## ##  ##   ##     ## ##               ######### ##        ##        
##       ##     ## ##     ## ##     ## ##        ##       ##   ###         ##    ## ##     ## ##   ##  ##     ## ##               ##     ## ##        ##        
########  #######  ##     ##  #######  ##        ######## ##    ## #######  ######   #######  ##    ##  #######  ######## ####### ##     ## ##        ##        
*/

typedef struct {
    char const* name;
    lua_Integer value;
}
IntConst;

static void l_regintconsts(lua_State* const L, IntConst const* const consts, size_t const count) {
    for (size_t i = 0; i < count; i++) {
        lua_pushinteger(L, consts[i].value);
        lua_setfield(L, -2, consts[i].name);
    }
}

LUAMOD_API int luaopen_sokol_app(lua_State* const L) {
    static luaL_Reg const functions[] = {
        {"isvalid", l_isvalid},
        {"width", l_width},
        {"height", l_height},
        {"color_format", l_color_format},
        {"depth_format", l_depth_format},
        {"sample_count", l_sample_count},
        {"high_dpi", l_high_dpi},
        {"dpi_scale", l_dpi_scale},
        {"show_keyboard", l_show_keyboard},
        {"keyboard_shown", l_keyboard_shown},
        {"is_fullscreen", l_is_fullscreen},
        {"toggle_fullscreen", l_toggle_fullscreen},
        {"show_mouse", l_show_mouse},
        {"mouse_shown", l_mouse_shown},
        {"lock_mouse", l_lock_mouse},
        {"mouse_locked", l_mouse_locked},
        {"query_desc", l_query_desc},
        {"request_quit", l_request_quit},
        {"cancel_quit", l_cancel_quit},
        {"quit", l_quit},
        {"consume_event", l_consume_event},
        {"frame_count", l_frame_count},
        {"frame_duration", l_frame_duration},
        {"set_clipboard_string", l_set_clipboard_string},
        {"get_clipboard_string", l_get_clipboard_string},
        {"set_window_title", l_set_window_title},
        {"set_icon", l_set_icon},
        {"get_num_dropped_files", l_get_num_dropped_files},
        {"get_dropped_file_path", l_get_dropped_file_path},
        {"gles2", l_gles2},
        {"html5_ask_leave_site", l_html5_ask_leave_site},
        {"html5_get_dropped_file_size", l_html5_get_dropped_file_size},

        {NULL, NULL}
    };

    luaL_newlib(L, functions);

    static IntConst const event_consts[] = {
        {"EVENTTYPE_INVALID", SAPP_EVENTTYPE_INVALID},
        {"EVENTTYPE_KEY_DOWN", SAPP_EVENTTYPE_KEY_DOWN},
        {"EVENTTYPE_KEY_UP", SAPP_EVENTTYPE_KEY_UP},
        {"EVENTTYPE_CHAR", SAPP_EVENTTYPE_CHAR},
        {"EVENTTYPE_MOUSE_DOWN", SAPP_EVENTTYPE_MOUSE_DOWN},
        {"EVENTTYPE_MOUSE_UP", SAPP_EVENTTYPE_MOUSE_UP},
        {"EVENTTYPE_MOUSE_SCROLL", SAPP_EVENTTYPE_MOUSE_SCROLL},
        {"EVENTTYPE_MOUSE_MOVE", SAPP_EVENTTYPE_MOUSE_MOVE},
        {"EVENTTYPE_MOUSE_ENTER", SAPP_EVENTTYPE_MOUSE_ENTER},
        {"EVENTTYPE_MOUSE_LEAVE", SAPP_EVENTTYPE_MOUSE_LEAVE},
        {"EVENTTYPE_TOUCHES_BEGAN", SAPP_EVENTTYPE_TOUCHES_BEGAN},
        {"EVENTTYPE_TOUCHES_MOVED", SAPP_EVENTTYPE_TOUCHES_MOVED},
        {"EVENTTYPE_TOUCHES_ENDED", SAPP_EVENTTYPE_TOUCHES_ENDED},
        {"EVENTTYPE_TOUCHES_CANCELLED", SAPP_EVENTTYPE_TOUCHES_CANCELLED},
        {"EVENTTYPE_RESIZED", SAPP_EVENTTYPE_RESIZED},
        {"EVENTTYPE_ICONIFIED", SAPP_EVENTTYPE_ICONIFIED},
        {"EVENTTYPE_RESTORED", SAPP_EVENTTYPE_RESTORED},
        {"EVENTTYPE_FOCUSED", SAPP_EVENTTYPE_FOCUSED},
        {"EVENTTYPE_UNFOCUSED", SAPP_EVENTTYPE_UNFOCUSED},
        {"EVENTTYPE_SUSPENDED", SAPP_EVENTTYPE_SUSPENDED},
        {"EVENTTYPE_RESUMED", SAPP_EVENTTYPE_RESUMED},
        {"EVENTTYPE_UPDATE_CURSOR", SAPP_EVENTTYPE_UPDATE_CURSOR},
        {"EVENTTYPE_QUIT_REQUESTED", SAPP_EVENTTYPE_QUIT_REQUESTED},
        {"EVENTTYPE_CLIPBOARD_PASTED", SAPP_EVENTTYPE_CLIPBOARD_PASTED},
        {"EVENTTYPE_FILES_DROPPED", SAPP_EVENTTYPE_FILES_DROPPED}
    };

    l_regintconsts(L, event_consts, sizeof(event_consts) / sizeof(event_consts[0]));

    static IntConst const keycode_consts[] = {
        {"KEYCODE_INVALID", SAPP_KEYCODE_INVALID},
        {"KEYCODE_SPACE", SAPP_KEYCODE_SPACE},
        {"KEYCODE_APOSTROPHE", SAPP_KEYCODE_APOSTROPHE},
        {"KEYCODE_COMMA", SAPP_KEYCODE_COMMA},
        {"KEYCODE_MINUS", SAPP_KEYCODE_MINUS},
        {"KEYCODE_PERIOD", SAPP_KEYCODE_PERIOD},
        {"KEYCODE_SLASH", SAPP_KEYCODE_SLASH},
        {"KEYCODE_0", SAPP_KEYCODE_0},
        {"KEYCODE_1", SAPP_KEYCODE_1},
        {"KEYCODE_2", SAPP_KEYCODE_2},
        {"KEYCODE_3", SAPP_KEYCODE_3},
        {"KEYCODE_4", SAPP_KEYCODE_4},
        {"KEYCODE_5", SAPP_KEYCODE_5},
        {"KEYCODE_6", SAPP_KEYCODE_6},
        {"KEYCODE_7", SAPP_KEYCODE_7},
        {"KEYCODE_8", SAPP_KEYCODE_8},
        {"KEYCODE_9", SAPP_KEYCODE_9},
        {"KEYCODE_SEMICOLON", SAPP_KEYCODE_SEMICOLON},
        {"KEYCODE_EQUAL", SAPP_KEYCODE_EQUAL},
        {"KEYCODE_A", SAPP_KEYCODE_A},
        {"KEYCODE_B", SAPP_KEYCODE_B},
        {"KEYCODE_C", SAPP_KEYCODE_C},
        {"KEYCODE_D", SAPP_KEYCODE_D},
        {"KEYCODE_E", SAPP_KEYCODE_E},
        {"KEYCODE_F", SAPP_KEYCODE_F},
        {"KEYCODE_G", SAPP_KEYCODE_G},
        {"KEYCODE_H", SAPP_KEYCODE_H},
        {"KEYCODE_I", SAPP_KEYCODE_I},
        {"KEYCODE_J", SAPP_KEYCODE_J},
        {"KEYCODE_K", SAPP_KEYCODE_K},
        {"KEYCODE_L", SAPP_KEYCODE_L},
        {"KEYCODE_M", SAPP_KEYCODE_M},
        {"KEYCODE_N", SAPP_KEYCODE_N},
        {"KEYCODE_O", SAPP_KEYCODE_O},
        {"KEYCODE_P", SAPP_KEYCODE_P},
        {"KEYCODE_Q", SAPP_KEYCODE_Q},
        {"KEYCODE_R", SAPP_KEYCODE_R},
        {"KEYCODE_S", SAPP_KEYCODE_S},
        {"KEYCODE_T", SAPP_KEYCODE_T},
        {"KEYCODE_U", SAPP_KEYCODE_U},
        {"KEYCODE_V", SAPP_KEYCODE_V},
        {"KEYCODE_W", SAPP_KEYCODE_W},
        {"KEYCODE_X", SAPP_KEYCODE_X},
        {"KEYCODE_Y", SAPP_KEYCODE_Y},
        {"KEYCODE_Z", SAPP_KEYCODE_Z},
        {"KEYCODE_LEFT_BRACKET", SAPP_KEYCODE_LEFT_BRACKET},
        {"KEYCODE_BACKSLASH", SAPP_KEYCODE_BACKSLASH},
        {"KEYCODE_RIGHT_BRACKET", SAPP_KEYCODE_RIGHT_BRACKET},
        {"KEYCODE_GRAVE_ACCENT", SAPP_KEYCODE_GRAVE_ACCENT},
        {"KEYCODE_WORLD_1", SAPP_KEYCODE_WORLD_1},
        {"KEYCODE_WORLD_2", SAPP_KEYCODE_WORLD_2},
        {"KEYCODE_ESCAPE", SAPP_KEYCODE_ESCAPE},
        {"KEYCODE_ENTER", SAPP_KEYCODE_ENTER},
        {"KEYCODE_TAB", SAPP_KEYCODE_TAB},
        {"KEYCODE_BACKSPACE", SAPP_KEYCODE_BACKSPACE},
        {"KEYCODE_INSERT", SAPP_KEYCODE_INSERT},
        {"KEYCODE_DELETE", SAPP_KEYCODE_DELETE},
        {"KEYCODE_RIGHT", SAPP_KEYCODE_RIGHT},
        {"KEYCODE_LEFT", SAPP_KEYCODE_LEFT},
        {"KEYCODE_DOWN", SAPP_KEYCODE_DOWN},
        {"KEYCODE_UP", SAPP_KEYCODE_UP},
        {"KEYCODE_PAGE_UP", SAPP_KEYCODE_PAGE_UP},
        {"KEYCODE_PAGE_DOWN", SAPP_KEYCODE_PAGE_DOWN},
        {"KEYCODE_HOME", SAPP_KEYCODE_HOME},
        {"KEYCODE_END", SAPP_KEYCODE_END},
        {"KEYCODE_CAPS_LOCK", SAPP_KEYCODE_CAPS_LOCK},
        {"KEYCODE_SCROLL_LOCK", SAPP_KEYCODE_SCROLL_LOCK},
        {"KEYCODE_NUM_LOCK", SAPP_KEYCODE_NUM_LOCK},
        {"KEYCODE_PRINT_SCREEN", SAPP_KEYCODE_PRINT_SCREEN},
        {"KEYCODE_PAUSE", SAPP_KEYCODE_PAUSE},
        {"KEYCODE_F1", SAPP_KEYCODE_F1},
        {"KEYCODE_F2", SAPP_KEYCODE_F2},
        {"KEYCODE_F3", SAPP_KEYCODE_F3},
        {"KEYCODE_F4", SAPP_KEYCODE_F4},
        {"KEYCODE_F5", SAPP_KEYCODE_F5},
        {"KEYCODE_F6", SAPP_KEYCODE_F6},
        {"KEYCODE_F7", SAPP_KEYCODE_F7},
        {"KEYCODE_F8", SAPP_KEYCODE_F8},
        {"KEYCODE_F9", SAPP_KEYCODE_F9},
        {"KEYCODE_F10", SAPP_KEYCODE_F10},
        {"KEYCODE_F11", SAPP_KEYCODE_F11},
        {"KEYCODE_F12", SAPP_KEYCODE_F12},
        {"KEYCODE_F13", SAPP_KEYCODE_F13},
        {"KEYCODE_F14", SAPP_KEYCODE_F14},
        {"KEYCODE_F15", SAPP_KEYCODE_F15},
        {"KEYCODE_F16", SAPP_KEYCODE_F16},
        {"KEYCODE_F17", SAPP_KEYCODE_F17},
        {"KEYCODE_F18", SAPP_KEYCODE_F18},
        {"KEYCODE_F19", SAPP_KEYCODE_F19},
        {"KEYCODE_F20", SAPP_KEYCODE_F20},
        {"KEYCODE_F21", SAPP_KEYCODE_F21},
        {"KEYCODE_F22", SAPP_KEYCODE_F22},
        {"KEYCODE_F23", SAPP_KEYCODE_F23},
        {"KEYCODE_F24", SAPP_KEYCODE_F24},
        {"KEYCODE_F25", SAPP_KEYCODE_F25},
        {"KEYCODE_KP_0", SAPP_KEYCODE_KP_0},
        {"KEYCODE_KP_1", SAPP_KEYCODE_KP_1},
        {"KEYCODE_KP_2", SAPP_KEYCODE_KP_2},
        {"KEYCODE_KP_3", SAPP_KEYCODE_KP_3},
        {"KEYCODE_KP_4", SAPP_KEYCODE_KP_4},
        {"KEYCODE_KP_5", SAPP_KEYCODE_KP_5},
        {"KEYCODE_KP_6", SAPP_KEYCODE_KP_6},
        {"KEYCODE_KP_7", SAPP_KEYCODE_KP_7},
        {"KEYCODE_KP_8", SAPP_KEYCODE_KP_8},
        {"KEYCODE_KP_9", SAPP_KEYCODE_KP_9},
        {"KEYCODE_KP_DECIMAL", SAPP_KEYCODE_KP_DECIMAL},
        {"KEYCODE_KP_DIVIDE", SAPP_KEYCODE_KP_DIVIDE},
        {"KEYCODE_KP_MULTIPLY", SAPP_KEYCODE_KP_MULTIPLY},
        {"KEYCODE_KP_SUBTRACT", SAPP_KEYCODE_KP_SUBTRACT},
        {"KEYCODE_KP_ADD", SAPP_KEYCODE_KP_ADD},
        {"KEYCODE_KP_ENTER", SAPP_KEYCODE_KP_ENTER},
        {"KEYCODE_KP_EQUAL", SAPP_KEYCODE_KP_EQUAL},
        {"KEYCODE_LEFT_SHIFT", SAPP_KEYCODE_LEFT_SHIFT},
        {"KEYCODE_LEFT_CONTROL", SAPP_KEYCODE_LEFT_CONTROL},
        {"KEYCODE_LEFT_ALT", SAPP_KEYCODE_LEFT_ALT},
        {"KEYCODE_LEFT_SUPER", SAPP_KEYCODE_LEFT_SUPER},
        {"KEYCODE_RIGHT_SHIFT", SAPP_KEYCODE_RIGHT_SHIFT},
        {"KEYCODE_RIGHT_CONTROL", SAPP_KEYCODE_RIGHT_CONTROL},
        {"KEYCODE_RIGHT_ALT", SAPP_KEYCODE_RIGHT_ALT},
        {"KEYCODE_RIGHT_SUPER", SAPP_KEYCODE_RIGHT_SUPER},
        {"KEYCODE_MENU", SAPP_KEYCODE_MENU}
    };

    l_regintconsts(L, keycode_consts, sizeof(keycode_consts) / sizeof(keycode_consts[0]));

    static IntConst const mousebutton_consts[] = {
        {"MOUSEBUTTON_LEFT", SAPP_MOUSEBUTTON_LEFT},
        {"MOUSEBUTTON_RIGHT", SAPP_MOUSEBUTTON_RIGHT},
        {"MOUSEBUTTON_MIDDLE", SAPP_MOUSEBUTTON_MIDDLE},
        {"MOUSEBUTTON_INVALID", SAPP_MOUSEBUTTON_INVALID}
    };

    l_regintconsts(L, mousebutton_consts, sizeof(mousebutton_consts) / sizeof(mousebutton_consts[0]));

    static IntConst const modifier_consts[] = {
        {"MODIFIER_SHIFT", SAPP_MODIFIER_SHIFT},
        {"MODIFIER_CTRL", SAPP_MODIFIER_CTRL},
        {"MODIFIER_ALT", SAPP_MODIFIER_ALT},
        {"MODIFIER_SUPER", SAPP_MODIFIER_SUPER},
        {"MODIFIER_LMB", SAPP_MODIFIER_LMB},
        {"MODIFIER_RMB", SAPP_MODIFIER_RMB},
        {"MODIFIER_MMB", SAPP_MODIFIER_MMB}
    };

    l_regintconsts(L, modifier_consts, sizeof(modifier_consts) / sizeof(modifier_consts[0]));

    return 1;
}

/*
 ######   #######  ##    ##  #######  ##               ##     ##    ###    #### ##    ## 
##    ## ##     ## ##   ##  ##     ## ##               ###   ###   ## ##    ##  ###   ## 
##       ##     ## ##  ##   ##     ## ##               #### ####  ##   ##   ##  ####  ## 
 ######  ##     ## #####    ##     ## ##               ## ### ## ##     ##  ##  ## ## ## 
      ## ##     ## ##  ##   ##     ## ##               ##     ## #########  ##  ##  #### 
##    ## ##     ## ##   ##  ##     ## ##               ##     ## ##     ##  ##  ##   ### 
 ######   #######  ##    ##  #######  ######## ####### ##     ## ##     ## #### ##    ## 
*/

typedef struct {
    char const* name;

    union {
        char const* source;
        lua_CFunction openf;
    };

    size_t length;
}
Module;

#define MODL(name, array) {name, {array}, sizeof(array)}
#define MODC(name, openf) {name, {(char*)openf}, 0}

static int l_searcher(lua_State* const L) {
    LUAMOD_API int luaopen_sapp(lua_State* L);

    static Module const modules[] = {
        MODC("sokol.app", luaopen_sokol_app),
    };

    char const* const modname = lua_tostring(L, 1);

    for (size_t i = 0; i < sizeof(modules) / sizeof(modules[0]); i++) {
        if (strcmp(modname, modules[i].name) == 0) {
            if (modules[i].length != 0) {
                int const res = luaL_loadbufferx(L, modules[i].source, modules[i].length, modname, "t");

                if (res != LUA_OK) {
                    return lua_error(L);
                }
            }
            else {
                lua_pushcfunction(L, modules[i].openf);
            }

            return 1;
        }
    }

    lua_pushfstring(L, "unknown module \"%s\"", modname);
    return 1;
}

static int l_panic(lua_State* const L) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    exit(EXIT_FAILURE);
}

static int l_traceback(lua_State* const L) {
    luaL_traceback(L, L, lua_tostring(L, -1), 1);
    return 1;
}

static int l_pcall(lua_State* const L, int const nargs, int const nres) {
    int const errndx = lua_gettop(L) - nargs;
    lua_pushcfunction(L, l_traceback);
    lua_insert(L, errndx);
  
    int const ret = lua_pcall(L, nargs, nres, errndx);
    lua_remove(L, errndx);

    return ret;
}

static void initcb(void* const user_data) {
    lua_State* const L = user_data;
    lua_rawgeti(L, LUA_REGISTRYINDEX, init_cb_ref);
    
    if (l_pcall(L, 0, 0) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        sapp_quit();
    }
}

static void framecb(void* const user_data) {
    lua_State* const L = user_data;
    lua_rawgeti(L, LUA_REGISTRYINDEX, frame_cb_ref);
    
    if (l_pcall(L, 0, 0) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        sapp_quit();
    }

    lua_gc(L, LUA_GCSTEP, 0);
}

static void cleanupcb(void* const user_data) {
    lua_State* const L = user_data;
    lua_rawgeti(L, LUA_REGISTRYINDEX, cleanup_cb_ref);
    
    if (l_pcall(L, 0, 0) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        sapp_quit();
    }
}

static void eventcb(sapp_event const* const event, void* const user_data) {
    lua_State* const L = user_data;
    lua_rawgeti(L, LUA_REGISTRYINDEX, event_cb_ref);
    event_push(L)->event = *event;

    if (l_pcall(L, 1, 0) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        sapp_quit();
    }
}

static void failcb(char const* error, void* const user_data) {
    lua_State* const L = user_data;
    lua_rawgeti(L, LUA_REGISTRYINDEX, fail_cb_ref);
    lua_pushstring(L, error);
    
    if (l_pcall(L, 1, 0) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        sapp_quit();
    }
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    lua_State* const L = luaL_newstate();
    lua_atpanic(L, l_panic);

    {
        static luaL_Reg const libs[] = {
            {LUA_GNAME, luaopen_base},
            {LUA_LOADLIBNAME, luaopen_package},
            {LUA_COLIBNAME, luaopen_coroutine},
            {LUA_TABLIBNAME, luaopen_table},
            {LUA_IOLIBNAME, luaopen_io},
            {LUA_OSLIBNAME, luaopen_os},
            {LUA_STRLIBNAME, luaopen_string},
            {LUA_MATHLIBNAME, luaopen_math},
            {LUA_UTF8LIBNAME, luaopen_utf8},
            {LUA_DBLIBNAME, luaopen_debug}
        };

        for (size_t i = 0; i < sizeof(libs) / sizeof(libs[0]); i++) {
            luaL_requiref(L, libs[i].name, libs[i].func, 1);
            lua_pop(L, 1);
        }
    }

    {
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "searchers");
        size_t const length = lua_rawlen(L, -1);

        lua_pushcfunction(L, l_searcher);
        lua_rawseti(L, -2, length + 1);

        lua_pop(L, 2);
    }

    {
        static char const* const luamain =
            "local sapp = require 'sokol.app'\n"
            "for k, v in pairs(sapp) do print(k, v) end\n"
            "local dummy = function() end\n"
            "local event_cb = function(event) print(event, event.type) end\n"
            "local fail_cb = function(msg) error('fail_cb', msg) end\n"
            "return {init_cb=dummy, frame_cb=dummy, cleanup_cb=dummy, event_cb=event_cb, fail_cb=dummy, width=320, height=240}\n";

        if (luaL_loadbufferx(L, luamain, strlen(luamain), "main.lua", "t") != LUA_OK) {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            exit(EXIT_FAILURE);
        }

        if (l_pcall(L, 0, 1) != LUA_OK) {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            exit(EXIT_FAILURE);
        }
    }

    sapp_desc desc;
    memset(&desc, 0, sizeof(desc));

    {
        desc.user_data = L;
        desc.init_userdata_cb = initcb;
        desc.frame_userdata_cb = framecb;
        desc.cleanup_userdata_cb = cleanupcb;
        desc.event_userdata_cb = eventcb;
        desc.fail_userdata_cb = failcb;

        if (lua_getfield(L, 1, "init_cb") != LUA_TNIL) {
            luaL_checktype(L, -1, LUA_TFUNCTION);
            init_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        if (lua_getfield(L, 1, "frame_cb") != LUA_TNIL) {
            luaL_checktype(L, -1, LUA_TFUNCTION);
            frame_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        if (lua_getfield(L, 1, "cleanup_cb") != LUA_TNIL) {
            luaL_checktype(L, -1, LUA_TFUNCTION);
            cleanup_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        if (lua_getfield(L, 1, "event_cb") != LUA_TNIL) {
            luaL_checktype(L, -1, LUA_TFUNCTION);
            event_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        if (lua_getfield(L, 1, "fail_cb") != LUA_TNIL) {
            luaL_checktype(L, -1, LUA_TFUNCTION);
            fail_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        if (lua_getfield(L, 1, "width") != LUA_TNIL) {
            desc.width = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "height") != LUA_TNIL) {
            desc.height = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "sample_count") != LUA_TNIL) {
            desc.sample_count = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "swap_interval") != LUA_TNIL) {
            desc.swap_interval = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "high_dpi") != LUA_TNIL) {
            desc.high_dpi = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "fullscreen") != LUA_TNIL) {
            desc.fullscreen = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "alpha") != LUA_TNIL) {
            desc.alpha = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "window_title") != LUA_TNIL) {
            size_t length = 0;
            char const* const title = luaL_checklstring(L, -1, &length);
            window_title = malloc(length + 1);
            strcpy(window_title, title);
            desc.window_title = title;
        }

        if (lua_getfield(L, 1, "user_cursor") != LUA_TNIL) {
            desc.user_cursor = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "enable_clipboard") != LUA_TNIL) {
            desc.enable_clipboard = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "clipboard_size") != LUA_TNIL) {
            desc.clipboard_size = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "enable_dragndrop") != LUA_TNIL) {
            desc.enable_clipboard = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "max_dropped_files") != LUA_TNIL) {
            desc.max_dropped_files = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "max_dropped_file_path_length") != LUA_TNIL) {
            desc.max_dropped_file_path_length = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "icon") != LUA_TNIL) {
            // TODO
        }

        if (lua_getfield(L, 1, "gl_force_gles2") != LUA_TNIL) {
            desc.gl_force_gles2 = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "win32_console_utf8") != LUA_TNIL) {
            desc.width = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "win32_console_create") != LUA_TNIL) {
            desc.win32_console_create = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "win32_console_attach") != LUA_TNIL) {
            desc.win32_console_attach = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "win32_console_attach") != LUA_TNIL) {
            size_t length = 0;
            char const* const name = luaL_checklstring(L, -1, &length);
            html5_canvas_name = malloc(length + 1);
            strcpy(html5_canvas_name, name);
            desc.html5_canvas_name = html5_canvas_name;
        }

        if (lua_getfield(L, 1, "html5_canvas_resize") != LUA_TNIL) {
            desc.html5_canvas_resize = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "html5_preserve_drawing_buffer") != LUA_TNIL) {
            desc.html5_preserve_drawing_buffer = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "html5_premultiplied_alpha") != LUA_TNIL) {
            desc.html5_premultiplied_alpha = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "html5_ask_leave_site") != LUA_TNIL) {
            desc.html5_ask_leave_site = lua_toboolean(L, -1);
        }

        if (lua_getfield(L, 1, "ios_keyboard_resizes_canvas") != LUA_TNIL) {
            desc.ios_keyboard_resizes_canvas = lua_toboolean(L, -1);
        }

        lua_settop(L, 0);
    }

    return desc;
}
