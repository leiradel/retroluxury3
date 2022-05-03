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
#include "lutil.h"

#include "app.lua.h"

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

static int event_touches(lua_State* const L) {
    Event const* const self = event_check(L, 1);
    lua_Integer const index = luaL_checkinteger(L, 2) - 1;

    if (index >= 0 && index < self->event.num_touches) {
        lua_pushinteger(L, self->event.touches[index].identifier);
        lua_pushnumber(L, self->event.touches[index].pos_x);
        lua_pushnumber(L, self->event.touches[index].pos_y);
        lua_pushboolean(L, self->event.touches[index].changed);
        return 4;
    }

    return 0;
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

        case DJB2HASH_C(0x444b846f): /* num_touches */
            lua_pushinteger(L, self->event.num_touches);
            return 1;

        case DJB2HASH_C(0xf08d0700): /* touches */
            lua_pushcfunction(L, event_touches);
            return 1;

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

static void fillicondesc(lua_State* const L, int const ndx, sapp_icon_desc* const icon) {
    for (lua_Integer i = 1; i < SAPP_MAX_ICONIMAGES; i++) {
        lua_geti(L, ndx, i);

        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            break;
        }

        lua_geti(L, -1, 1);
        icon->images[i - 1].width = luaL_checkinteger(L, -1);

        lua_geti(L, -2, 2);
        icon->images[i - 1].height = luaL_checkinteger(L, -1);

        lua_geti(L, -3, 3);
        icon->images[i - 1].pixels.ptr = luaL_checklstring(L, -1, &icon->images[i - 1].pixels.size);

        lua_pop(L, 4);
    }
}

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
    sapp_icon_desc icon;
    memset(&icon, 0, sizeof(icon));

    fillicondesc(L, 1, &icon);
    sapp_set_icon(&icon);

    return 0;
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

    static lutil_IntConst const event_consts[] = {
        {"INVALID", SAPP_EVENTTYPE_INVALID},
        {"KEY_DOWN", SAPP_EVENTTYPE_KEY_DOWN},
        {"KEY_UP", SAPP_EVENTTYPE_KEY_UP},
        {"CHAR", SAPP_EVENTTYPE_CHAR},
        {"MOUSE_DOWN", SAPP_EVENTTYPE_MOUSE_DOWN},
        {"MOUSE_UP", SAPP_EVENTTYPE_MOUSE_UP},
        {"MOUSE_SCROLL", SAPP_EVENTTYPE_MOUSE_SCROLL},
        {"MOUSE_MOVE", SAPP_EVENTTYPE_MOUSE_MOVE},
        {"MOUSE_ENTER", SAPP_EVENTTYPE_MOUSE_ENTER},
        {"MOUSE_LEAVE", SAPP_EVENTTYPE_MOUSE_LEAVE},
        {"TOUCHES_BEGAN", SAPP_EVENTTYPE_TOUCHES_BEGAN},
        {"TOUCHES_MOVED", SAPP_EVENTTYPE_TOUCHES_MOVED},
        {"TOUCHES_ENDED", SAPP_EVENTTYPE_TOUCHES_ENDED},
        {"TOUCHES_CANCELLED", SAPP_EVENTTYPE_TOUCHES_CANCELLED},
        {"RESIZED", SAPP_EVENTTYPE_RESIZED},
        {"ICONIFIED", SAPP_EVENTTYPE_ICONIFIED},
        {"RESTORED", SAPP_EVENTTYPE_RESTORED},
        {"FOCUSED", SAPP_EVENTTYPE_FOCUSED},
        {"UNFOCUSED", SAPP_EVENTTYPE_UNFOCUSED},
        {"SUSPENDED", SAPP_EVENTTYPE_SUSPENDED},
        {"RESUMED", SAPP_EVENTTYPE_RESUMED},
        {"UPDATE_CURSOR", SAPP_EVENTTYPE_UPDATE_CURSOR},
        {"QUIT_REQUESTED", SAPP_EVENTTYPE_QUIT_REQUESTED},
        {"CLIPBOARD_PASTED", SAPP_EVENTTYPE_CLIPBOARD_PASTED},
        {"FILES_DROPPED", SAPP_EVENTTYPE_FILES_DROPPED}
    };

    lutil_regintconsts(L, "eventtype", event_consts, sizeof(event_consts) / sizeof(event_consts[0]));

    static lutil_IntConst const keycode_consts[] = {
        {"INVALID", SAPP_KEYCODE_INVALID},
        {"SPACE", SAPP_KEYCODE_SPACE},
        {"APOSTROPHE", SAPP_KEYCODE_APOSTROPHE},
        {"COMMA", SAPP_KEYCODE_COMMA},
        {"MINUS", SAPP_KEYCODE_MINUS},
        {"PERIOD", SAPP_KEYCODE_PERIOD},
        {"SLASH", SAPP_KEYCODE_SLASH},
        {"0", SAPP_KEYCODE_0},
        {"1", SAPP_KEYCODE_1},
        {"2", SAPP_KEYCODE_2},
        {"3", SAPP_KEYCODE_3},
        {"4", SAPP_KEYCODE_4},
        {"5", SAPP_KEYCODE_5},
        {"6", SAPP_KEYCODE_6},
        {"7", SAPP_KEYCODE_7},
        {"8", SAPP_KEYCODE_8},
        {"9", SAPP_KEYCODE_9},
        {"SEMICOLON", SAPP_KEYCODE_SEMICOLON},
        {"EQUAL", SAPP_KEYCODE_EQUAL},
        {"A", SAPP_KEYCODE_A},
        {"B", SAPP_KEYCODE_B},
        {"C", SAPP_KEYCODE_C},
        {"D", SAPP_KEYCODE_D},
        {"E", SAPP_KEYCODE_E},
        {"F", SAPP_KEYCODE_F},
        {"G", SAPP_KEYCODE_G},
        {"H", SAPP_KEYCODE_H},
        {"I", SAPP_KEYCODE_I},
        {"J", SAPP_KEYCODE_J},
        {"K", SAPP_KEYCODE_K},
        {"L", SAPP_KEYCODE_L},
        {"M", SAPP_KEYCODE_M},
        {"N", SAPP_KEYCODE_N},
        {"O", SAPP_KEYCODE_O},
        {"P", SAPP_KEYCODE_P},
        {"Q", SAPP_KEYCODE_Q},
        {"R", SAPP_KEYCODE_R},
        {"S", SAPP_KEYCODE_S},
        {"T", SAPP_KEYCODE_T},
        {"U", SAPP_KEYCODE_U},
        {"V", SAPP_KEYCODE_V},
        {"W", SAPP_KEYCODE_W},
        {"X", SAPP_KEYCODE_X},
        {"Y", SAPP_KEYCODE_Y},
        {"Z", SAPP_KEYCODE_Z},
        {"LEFT_BRACKET", SAPP_KEYCODE_LEFT_BRACKET},
        {"BACKSLASH", SAPP_KEYCODE_BACKSLASH},
        {"RIGHT_BRACKET", SAPP_KEYCODE_RIGHT_BRACKET},
        {"GRAVE_ACCENT", SAPP_KEYCODE_GRAVE_ACCENT},
        {"WORLD_1", SAPP_KEYCODE_WORLD_1},
        {"WORLD_2", SAPP_KEYCODE_WORLD_2},
        {"ESCAPE", SAPP_KEYCODE_ESCAPE},
        {"ENTER", SAPP_KEYCODE_ENTER},
        {"TAB", SAPP_KEYCODE_TAB},
        {"BACKSPACE", SAPP_KEYCODE_BACKSPACE},
        {"INSERT", SAPP_KEYCODE_INSERT},
        {"DELETE", SAPP_KEYCODE_DELETE},
        {"RIGHT", SAPP_KEYCODE_RIGHT},
        {"LEFT", SAPP_KEYCODE_LEFT},
        {"DOWN", SAPP_KEYCODE_DOWN},
        {"UP", SAPP_KEYCODE_UP},
        {"PAGE_UP", SAPP_KEYCODE_PAGE_UP},
        {"PAGE_DOWN", SAPP_KEYCODE_PAGE_DOWN},
        {"HOME", SAPP_KEYCODE_HOME},
        {"END", SAPP_KEYCODE_END},
        {"CAPS_LOCK", SAPP_KEYCODE_CAPS_LOCK},
        {"SCROLL_LOCK", SAPP_KEYCODE_SCROLL_LOCK},
        {"NUM_LOCK", SAPP_KEYCODE_NUM_LOCK},
        {"PRINT_SCREEN", SAPP_KEYCODE_PRINT_SCREEN},
        {"PAUSE", SAPP_KEYCODE_PAUSE},
        {"F1", SAPP_KEYCODE_F1},
        {"F2", SAPP_KEYCODE_F2},
        {"F3", SAPP_KEYCODE_F3},
        {"F4", SAPP_KEYCODE_F4},
        {"F5", SAPP_KEYCODE_F5},
        {"F6", SAPP_KEYCODE_F6},
        {"F7", SAPP_KEYCODE_F7},
        {"F8", SAPP_KEYCODE_F8},
        {"F9", SAPP_KEYCODE_F9},
        {"F10", SAPP_KEYCODE_F10},
        {"F11", SAPP_KEYCODE_F11},
        {"F12", SAPP_KEYCODE_F12},
        {"F13", SAPP_KEYCODE_F13},
        {"F14", SAPP_KEYCODE_F14},
        {"F15", SAPP_KEYCODE_F15},
        {"F16", SAPP_KEYCODE_F16},
        {"F17", SAPP_KEYCODE_F17},
        {"F18", SAPP_KEYCODE_F18},
        {"F19", SAPP_KEYCODE_F19},
        {"F20", SAPP_KEYCODE_F20},
        {"F21", SAPP_KEYCODE_F21},
        {"F22", SAPP_KEYCODE_F22},
        {"F23", SAPP_KEYCODE_F23},
        {"F24", SAPP_KEYCODE_F24},
        {"F25", SAPP_KEYCODE_F25},
        {"KP_0", SAPP_KEYCODE_KP_0},
        {"KP_1", SAPP_KEYCODE_KP_1},
        {"KP_2", SAPP_KEYCODE_KP_2},
        {"KP_3", SAPP_KEYCODE_KP_3},
        {"KP_4", SAPP_KEYCODE_KP_4},
        {"KP_5", SAPP_KEYCODE_KP_5},
        {"KP_6", SAPP_KEYCODE_KP_6},
        {"KP_7", SAPP_KEYCODE_KP_7},
        {"KP_8", SAPP_KEYCODE_KP_8},
        {"KP_9", SAPP_KEYCODE_KP_9},
        {"KP_DECIMAL", SAPP_KEYCODE_KP_DECIMAL},
        {"KP_DIVIDE", SAPP_KEYCODE_KP_DIVIDE},
        {"KP_MULTIPLY", SAPP_KEYCODE_KP_MULTIPLY},
        {"KP_SUBTRACT", SAPP_KEYCODE_KP_SUBTRACT},
        {"KP_ADD", SAPP_KEYCODE_KP_ADD},
        {"KP_ENTER", SAPP_KEYCODE_KP_ENTER},
        {"KP_EQUAL", SAPP_KEYCODE_KP_EQUAL},
        {"LEFT_SHIFT", SAPP_KEYCODE_LEFT_SHIFT},
        {"LEFT_CONTROL", SAPP_KEYCODE_LEFT_CONTROL},
        {"LEFT_ALT", SAPP_KEYCODE_LEFT_ALT},
        {"LEFT_SUPER", SAPP_KEYCODE_LEFT_SUPER},
        {"RIGHT_SHIFT", SAPP_KEYCODE_RIGHT_SHIFT},
        {"RIGHT_CONTROL", SAPP_KEYCODE_RIGHT_CONTROL},
        {"RIGHT_ALT", SAPP_KEYCODE_RIGHT_ALT},
        {"RIGHT_SUPER", SAPP_KEYCODE_RIGHT_SUPER},
        {"MENU", SAPP_KEYCODE_MENU}
    };

    lutil_regintconsts(L, "keycode", keycode_consts, sizeof(keycode_consts) / sizeof(keycode_consts[0]));

    static lutil_IntConst const mousebutton_consts[] = {
        {"LEFT", SAPP_MOUSEBUTTON_LEFT},
        {"RIGHT", SAPP_MOUSEBUTTON_RIGHT},
        {"MIDDLE", SAPP_MOUSEBUTTON_MIDDLE},
        {"INVALID", SAPP_MOUSEBUTTON_INVALID}
    };

    lutil_regintconsts(L, "mousebutton", mousebutton_consts, sizeof(mousebutton_consts) / sizeof(mousebutton_consts[0]));

    static lutil_IntConst const modifier_consts[] = {
        {"SHIFT", SAPP_MODIFIER_SHIFT},
        {"CTRL", SAPP_MODIFIER_CTRL},
        {"ALT", SAPP_MODIFIER_ALT},
        {"SUPER", SAPP_MODIFIER_SUPER},
        {"LMB", SAPP_MODIFIER_LMB},
        {"RMB", SAPP_MODIFIER_RMB},
        {"MMB", SAPP_MODIFIER_MMB}
    };

    lutil_regintconsts(L, "modifier", modifier_consts, sizeof(modifier_consts) / sizeof(modifier_consts[0]));

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
    LUAMOD_API int luaopen_sokol_fetch(lua_State* const L);
    LUAMOD_API int luaopen_rl3_image(lua_State* const L);


    static Module const modules[] = {
        MODC("sokol.app", luaopen_sokol_app),
        MODC("sokol.fetch", luaopen_sokol_fetch),
        MODC("rl3.image", luaopen_rl3_image),
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
#ifndef NDEBUG
    fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

    exit(EXIT_FAILURE);
}

static void initcb(void* const user_data) {
    if (init_cb_ref != LUA_NOREF) {
        lua_State* const L = user_data;
        lua_rawgeti(L, LUA_REGISTRYINDEX, init_cb_ref);
        
        if (lutil_pcall(L, 0, 0) != LUA_OK) {
#ifndef NDEBUG
            fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

            sapp_quit();
        }
    }
}

static void framecb(void* const user_data) {
    lua_State* const L = user_data;

    if (frame_cb_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, frame_cb_ref);
        
        if (lutil_pcall(L, 0, 0) != LUA_OK) {
#ifndef NDEBUG
            fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

            sapp_quit();
        }
    }

    lua_gc(L, LUA_GCSTEP, 0);
}

static void cleanupcb(void* const user_data) {
    if (cleanup_cb_ref != LUA_NOREF) {
        lua_State* const L = user_data;
        lua_rawgeti(L, LUA_REGISTRYINDEX, cleanup_cb_ref);
        
        if (lutil_pcall(L, 0, 0) != LUA_OK) {
#ifndef NDEBUG
            fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

            sapp_quit();
        }
    }
}

static void eventcb(sapp_event const* const event, void* const user_data) {
    if (event_cb_ref != LUA_NOREF) {
        lua_State* const L = user_data;
        lua_rawgeti(L, LUA_REGISTRYINDEX, event_cb_ref);
        event_push(L)->event = *event;

        if (lutil_pcall(L, 1, 0) != LUA_OK) {
#ifndef NDEBUG
            fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

            sapp_quit();
        }
    }
}

static void failcb(char const* error, void* const user_data) {
    if (fail_cb_ref != LUA_NOREF) {
        lua_State* const L = user_data;
        lua_rawgeti(L, LUA_REGISTRYINDEX, fail_cb_ref);
        lua_pushstring(L, error);
        
        if (lutil_pcall(L, 1, 0) != LUA_OK) {
#ifndef NDEBUG
            fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

            sapp_quit();
        }
    }
}

static int lua_main(lua_State* const L) {
    sapp_desc* const desc = lua_touserdata(L, 1);
    memset(desc, 0, sizeof(*desc));

    desc->user_data = L;
    desc->init_userdata_cb = initcb;
    desc->frame_userdata_cb = framecb;
    desc->cleanup_userdata_cb = cleanupcb;
    desc->event_userdata_cb = eventcb;
    desc->fail_userdata_cb = failcb;

    if (lua_getfield(L, 2, "init_cb") != LUA_TNIL) {
        init_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    if (lua_getfield(L, 2, "frame_cb") != LUA_TNIL) {
        frame_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    if (lua_getfield(L, 2, "cleanup_cb") != LUA_TNIL) {
        cleanup_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    if (lua_getfield(L, 2, "event_cb") != LUA_TNIL) {
        event_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    if (lua_getfield(L, 2, "fail_cb") != LUA_TNIL) {
        fail_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    if (lua_getfield(L, 2, "width") != LUA_TNIL) {
        desc->width = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 2, "height") != LUA_TNIL) {
        desc->height = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 2, "sample_count") != LUA_TNIL) {
        desc->sample_count = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 2, "swap_interval") != LUA_TNIL) {
        desc->swap_interval = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 2, "high_dpi") != LUA_TNIL) {
        desc->high_dpi = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "fullscreen") != LUA_TNIL) {
        desc->fullscreen = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "alpha") != LUA_TNIL) {
        desc->alpha = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "window_title") != LUA_TNIL) {
        size_t length = 0;
        char const* const title = luaL_checklstring(L, -1, &length);
        window_title = malloc(length + 1);
        strcpy(window_title, title);
        desc->window_title = window_title;
    }

    if (lua_getfield(L, 2, "user_cursor") != LUA_TNIL) {
        desc->user_cursor = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "enable_clipboard") != LUA_TNIL) {
        desc->enable_clipboard = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "clipboard_size") != LUA_TNIL) {
        desc->clipboard_size = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 2, "enable_dragndrop") != LUA_TNIL) {
        desc->enable_clipboard = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "max_dropped_files") != LUA_TNIL) {
        desc->max_dropped_files = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 2, "max_dropped_file_path_length") != LUA_TNIL) {
        desc->max_dropped_file_path_length = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 2, "icon") != LUA_TNIL) {
        fillicondesc(L, -1, &desc->icon);
    }
    else {
        desc->icon.sokol_default = true;
    }

    if (lua_getfield(L, 2, "gl_force_gles2") != LUA_TNIL) {
        desc->gl_force_gles2 = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "win32_console_utf8") != LUA_TNIL) {
        desc->width = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 2, "win32_console_create") != LUA_TNIL) {
        desc->win32_console_create = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "win32_console_attach") != LUA_TNIL) {
        desc->win32_console_attach = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "html5_canvas_name") != LUA_TNIL) {
        size_t length = 0;
        char const* const name = luaL_checklstring(L, -1, &length);
        html5_canvas_name = malloc(length + 1);
        strcpy(html5_canvas_name, name);
        desc->html5_canvas_name = html5_canvas_name;
    }

    if (lua_getfield(L, 2, "html5_canvas_resize") != LUA_TNIL) {
        desc->html5_canvas_resize = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "html5_preserve_drawing_buffer") != LUA_TNIL) {
        desc->html5_preserve_drawing_buffer = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "html5_premultiplied_alpha") != LUA_TNIL) {
        desc->html5_premultiplied_alpha = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "html5_ask_leave_site") != LUA_TNIL) {
        desc->html5_ask_leave_site = lua_toboolean(L, -1);
    }

    if (lua_getfield(L, 2, "ios_keyboard_resizes_canvas") != LUA_TNIL) {
        desc->ios_keyboard_resizes_canvas = lua_toboolean(L, -1);
    }

    return 0;
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    lua_State* const L = luaL_newstate();
    lua_atpanic(L, l_panic);

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

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchers");
    size_t const length = lua_rawlen(L, -1);

    lua_pushcfunction(L, l_searcher);
    lua_rawseti(L, -2, length + 1);

    lua_pop(L, 2);

    // Push the main Lua function
    lua_pushcfunction(L, lua_main);

    // Push a sapp_desc as the first argument
    sapp_desc desc;
    lua_pushlightuserdata(L, &desc);

    // Load the bootstrap script
    if (luaL_loadbufferx(L, app_lua, sizeof(app_lua) / sizeof(app_lua[0]), "bootstrap.lua", "t") != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        exit(EXIT_FAILURE);
    }

    // Call the compiled script, it will return the main.lua script as a compiled chunk
    if (lutil_pcall(L, 0, 1) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        exit(EXIT_FAILURE);
    }

    // Call the compiled main.lua, it will return a table with the sapp_desc fields
    if (lutil_pcall(L, 0, 1) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        exit(EXIT_FAILURE);
    }

    // Call lua_main with the sapp_desc pointer and the table
    if (lutil_pcall(L, 2, 0) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif

        exit(EXIT_FAILURE);
    }

    return desc;
}
