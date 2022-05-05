#include <stdint.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include <sokol_fetch.h>

#include "djb2.h"
#include "lutil.h"

#include "fetch.lua.h"

/*
 ######  ######## ######## ########  ######  ##     ##         ########  ########  ######   ######          ######## 
##    ## ##       ##          ##    ##    ## ##     ##         ##     ## ##       ##    ## ##    ##            ##    
##       ##       ##          ##    ##       ##     ##         ##     ## ##       ##       ##                  ##    
 ######  ######   ######      ##    ##       #########         ##     ## ######    ######  ##                  ##    
      ## ##       ##          ##    ##       ##     ##         ##     ## ##             ## ##                  ##    
##    ## ##       ##          ##    ##    ## ##     ##         ##     ## ##       ##    ## ##    ##            ##    
 ######  ##       ########    ##     ######  ##     ## ####### ########  ########  ######   ######  #######    ##    
*/

#define SFETCH_DESC_MT "sfetch_desc_t"

typedef struct {
    sfetch_desc_t desc;
}
Desc;

static Desc* desc_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SFETCH_DESC_MT);
}

static int desc_index(lua_State* const L) {
    Desc const* const self = desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x7ba46c86): /* max_requests */
            lua_pushinteger(L, self->desc.max_requests);
            return 1;

        case DJB2HASH_C(0xd4a870e0): /* num_channels */
            lua_pushinteger(L, self->desc.num_channels);
            return 1;

        case DJB2HASH_C(0xa5aabe67): /* num_lanes */
            lua_pushinteger(L, self->desc.num_lanes);
            return 1;
    }

    return luaL_error(L, "unknown field %s in %s", key, SFETCH_DESC_MT);
}

#ifndef NDEBUG
static int desc_newindex(lua_State* const L) {
    desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x7ba46c86): /* max_requests */
        case DJB2HASH_C(0xd4a870e0): /* num_channels */
        case DJB2HASH_C(0xa5aabe67): /* num_lanes */
            return luaL_error(L, "cannot change field %s in %s, object is read-only", key, SFETCH_DESC_MT);
    }

    return luaL_error(L, "unknown field %s in %s", key, SFETCH_DESC_MT);
}
#endif

static Desc* desc_push(lua_State* const L) {
    Desc* const self = lua_newuserdata(L, sizeof(*self));

    if (luaL_newmetatable(L, SFETCH_DESC_MT) != 0) {
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
 ######  ######## ######## ########  ######  ##     ##         ##     ##    ###    ##    ## ########  ##       ########         ######## 
##    ## ##       ##          ##    ##    ## ##     ##         ##     ##   ## ##   ###   ## ##     ## ##       ##                  ##    
##       ##       ##          ##    ##       ##     ##         ##     ##  ##   ##  ####  ## ##     ## ##       ##                  ##    
 ######  ######   ######      ##    ##       #########         ######### ##     ## ## ## ## ##     ## ##       ######              ##    
      ## ##       ##          ##    ##       ##     ##         ##     ## ######### ##  #### ##     ## ##       ##                  ##    
##    ## ##       ##          ##    ##    ## ##     ##         ##     ## ##     ## ##   ### ##     ## ##       ##                  ##    
 ######  ##       ########    ##     ######  ##     ## ####### ##     ## ##     ## ##    ## ########  ######## ######## #######    ##    
*/

#define SFETCH_HANDLE_MT "sfetch_handle_t"

typedef struct {
    sfetch_handle_t handle;
    uint8_t buffer[0];
}
Handle;

static Handle* handle_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SFETCH_HANDLE_MT);
}

static int l_handle_valid(lua_State* const L) {
    Handle const* const self = handle_check(L, 1);
    bool const valid = sfetch_handle_valid(self->handle);
    lua_pushboolean(L, valid);
    return 1;
}

static int l_handle_cancel(lua_State* const L) {
    Handle const* const self = handle_check(L, 1);
    sfetch_cancel(self->handle);
    return 0;
}

static int l_handle_pause(lua_State* const L) {
    Handle const* const self = handle_check(L, 1);
    sfetch_pause(self->handle);
    return 0;
}

static int l_handle_continue(lua_State* const L) {
    Handle const* const self = handle_check(L, 1);
    sfetch_continue(self->handle);
    return 0;
}

static int handle_eq(lua_State* const L) {
    Handle const* const self = handle_check(L, 1);
    Handle const* const other = handle_check(L, 2);

    bool const equal = self->handle.id == other->handle.id;
    lua_pushboolean(L, equal);
    return 1;
}

static Handle* handle_push(lua_State* const L, size_t const buffer_size) {
    Handle* const self = lua_newuserdata(L, sizeof(*self) + buffer_size);

    if (luaL_newmetatable(L, SFETCH_HANDLE_MT) != 0) {
        static luaL_Reg const methods[] = {
            {"valid", l_handle_valid},
            {"cancel", l_handle_cancel},
            {"pause", l_handle_pause},
            {"continue", l_handle_continue},
            {NULL, NULL}
        };

        luaL_newlib(L, methods);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, handle_eq);
        lua_setfield(L, -2, "__eq");
    }

    lua_setmetatable(L, -2);
    return self;
}

/*
 ######  ######## ######## ########  ######  ##     ##         ########  ########  ######  ########   #######  ##    ##  ######  ########         ######## 
##    ## ##       ##          ##    ##    ## ##     ##         ##     ## ##       ##    ## ##     ## ##     ## ###   ## ##    ## ##                  ##    
##       ##       ##          ##    ##       ##     ##         ##     ## ##       ##       ##     ## ##     ## ####  ## ##       ##                  ##    
 ######  ######   ######      ##    ##       #########         ########  ######    ######  ########  ##     ## ## ## ##  ######  ######              ##    
      ## ##       ##          ##    ##       ##     ##         ##   ##   ##             ## ##        ##     ## ##  ####       ## ##                  ##    
##    ## ##       ##          ##    ##    ## ##     ##         ##    ##  ##       ##    ## ##        ##     ## ##   ### ##    ## ##                  ##    
 ######  ##       ########    ##     ######  ##     ## ####### ##     ## ########  ######  ##         #######  ##    ##  ######  ######## #######    ##    
*/

#define SFETCH_RESPONSE_MT "sfetch_response_t"

typedef struct {
    sfetch_response_t response;
}
Response;

static Response* response_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SFETCH_RESPONSE_MT);
}

static int response_index(lua_State* const L) {
    Response const* const self = response_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x0190d871): /* handle */
            handle_push(L, 0)->handle = self->response.handle;
            return 1;

        case DJB2HASH_C(0x4a95afde): /* dispatched */
            lua_pushboolean(L, self->response.dispatched);
            return 1;

        case DJB2HASH_C(0xa3780598): /* fetched */
            lua_pushboolean(L, self->response.fetched);
            return 1;

        case DJB2HASH_C(0x143e3307): /* paused */
            lua_pushboolean(L, self->response.paused);
            return 1;

        case DJB2HASH_C(0x38d5604f): /* finished */
            lua_pushboolean(L, self->response.finished);
            return 1;

        case DJB2HASH_C(0xfce3ea6a): /* failed */
            lua_pushboolean(L, self->response.failed);
            return 1;

        case DJB2HASH_C(0x85037520): /* cancelled */
            lua_pushboolean(L, self->response.cancelled);
            return 1;

        case DJB2HASH_C(0x53eff629): /* error_code */
            lua_pushinteger(L, self->response.error_code);
            return 1;

        case DJB2HASH_C(0xc237195e): /* channel */
            lua_pushinteger(L, self->response.channel);
            return 1;

        case DJB2HASH_C(0x7c99f3a5): /* lane */
            lua_pushinteger(L, self->response.lane);
            return 1;

        case DJB2HASH_C(0x7c9c25f2): /* path */
            lua_pushstring(L, self->response.path);
            return 1;

        case DJB2HASH_C(0x37ef9cbe): /* fetched_offset */
            lua_pushinteger(L, self->response.fetched_offset);
            return 1;

        case DJB2HASH_C(0xa1cb3ad2): /* fetched_size */
            lua_pushinteger(L, self->response.fetched_size);
            return 1;

        case DJB2HASH_C(0xf4f7735f): /* buffer */
            lua_pushlstring(L, self->response.buffer_ptr, self->response.fetched_size);
            return 1;
    }

    return luaL_error(L, "unknown field %s in %s", key, SFETCH_RESPONSE_MT);
}

#ifndef NDEBUG
static int response_newindex(lua_State* const L) {
    response_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x0190d871): /* handle */
        case DJB2HASH_C(0x4a95afde): /* dispatched */
        case DJB2HASH_C(0xa3780598): /* fetched */
        case DJB2HASH_C(0x143e3307): /* paused */
        case DJB2HASH_C(0x38d5604f): /* finished */
        case DJB2HASH_C(0xfce3ea6a): /* failed */
        case DJB2HASH_C(0x85037520): /* cancelled */
        case DJB2HASH_C(0x53eff629): /* error_code */
        case DJB2HASH_C(0xc237195e): /* channel */
        case DJB2HASH_C(0x7c99f3a5): /* lane */
        case DJB2HASH_C(0x7c9c25f2): /* path */
        case DJB2HASH_C(0x37ef9cbe): /* fetched_offset */
        case DJB2HASH_C(0xa1cb3ad2): /* fetched_size */
        case DJB2HASH_C(0xf4f7735f): /* buffer */
            return luaL_error(L, "cannot change field %s in %s, object is read-only", key, SFETCH_RESPONSE_MT);
    }

    return luaL_error(L, "unknown field %s in %s", key, SFETCH_RESPONSE_MT);
}
#endif

static Response* response_push(lua_State* const L) {
    Response* const self = lua_newuserdata(L, sizeof(*self));

    if (luaL_newmetatable(L, SFETCH_RESPONSE_MT) != 0) {
        lua_pushcfunction(L, response_index);
        lua_setfield(L, -2, "__index");

#ifndef NDEBUG
        lua_pushcfunction(L, response_newindex);
        lua_setfield(L, -2, "__newindex");
#endif
    }

    lua_setmetatable(L, -2);
    return self;
}

/*
##       ##     ##    ###     #######  ########  ######## ##    ##          ######   #######  ##    ##  #######  ##               ######## ######## ########  ######  ##     ## 
##       ##     ##   ## ##   ##     ## ##     ## ##       ###   ##         ##    ## ##     ## ##   ##  ##     ## ##               ##       ##          ##    ##    ## ##     ## 
##       ##     ##  ##   ##  ##     ## ##     ## ##       ####  ##         ##       ##     ## ##  ##   ##     ## ##               ##       ##          ##    ##       ##     ## 
##       ##     ## ##     ## ##     ## ########  ######   ## ## ##          ######  ##     ## #####    ##     ## ##               ######   ######      ##    ##       ######### 
##       ##     ## ######### ##     ## ##        ##       ##  ####               ## ##     ## ##  ##   ##     ## ##               ##       ##          ##    ##       ##     ## 
##       ##     ## ##     ## ##     ## ##        ##       ##   ###         ##    ## ##     ## ##   ##  ##     ## ##               ##       ##          ##    ##    ## ##     ## 
########  #######  ##     ##  #######  ##        ######## ##    ## #######  ######   #######  ##    ##  #######  ######## ####### ##       ########    ##     ######  ##     ## 
*/

static int l_setup(lua_State* const L) {
    sfetch_desc_t desc;
    memset(&desc, 0, sizeof(desc));

    if (!lua_isnone(L, 1)) {
        if (lua_getfield(L, 1, "max_requests") != LUA_TNIL) {
            desc.max_requests = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "num_channels") != LUA_TNIL) {
            desc.num_channels = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "num_lanes") != LUA_TNIL) {
            desc.num_lanes = luaL_checkinteger(L, -1);
        }
    }

    sfetch_setup(&desc);
    lua_settop(L, 0);
    return 0;
}

static int l_shutdown(lua_State* const L) {
    (void)L;
    sfetch_shutdown();
    return 0;
}

static int l_valid(lua_State* const L) {
    bool const valid = sfetch_valid();
    lua_pushboolean(L, valid);
    return 1;
}

static int l_desc(lua_State* const L) {
    Desc* const self = desc_push(L);
    self->desc = sfetch_desc();
    return 1;
}

static int l_max_path(lua_State* const L) {
    int const max_path = sfetch_max_path();
    lua_pushinteger(L, max_path);
    return 1;
}

typedef struct {
    lua_State* const L;
    int const handle_ref;
    int const callback_ref;
}
Userdata;

static void callback(sfetch_response_t const* const response) {
    Userdata const* const ud = response->user_data;
    lua_State* const L = ud->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, ud->callback_ref);
    response_push(L)->response = *response;

    lua_call(L, 1, 0);

    if (response->finished) {
        luaL_unref(L, LUA_REGISTRYINDEX, ud->handle_ref);
    }
}

static int l_send(lua_State* const L) {
    sfetch_request_t request;
    memset(&request, 0, sizeof(request));

    if (lua_getfield(L, 1, "channel") != LUA_TNIL) {
        request.channel = luaL_checkinteger(L, -1);
    }

    if (lua_getfield(L, 1, "path") == LUA_TNIL) {
        return luaL_error(L, "missing required field 'path'");
    }

    request.path = luaL_checkstring(L, -1);

    if (lua_getfield(L, 1, "path") != LUA_TNIL) {
        request.path = luaL_checkstring(L, -1);
    }

    request.callback = callback;

    if (lua_getfield(L, 1, "callback") == LUA_TNIL) {
        return luaL_error(L, "missing required field 'callback'");
    }

    luaL_checktype(L, -1, LUA_TFUNCTION);
    int const callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (lua_getfield(L, 1, "chunk_size") != LUA_TNIL) {
        request.chunk_size = luaL_checkinteger(L, -1);
    }

    Handle* const self = handle_push(L, request.chunk_size);

    lua_pushvalue(L, -1);
    int const handle_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    Userdata ud = {L, handle_ref, callback_ref};
    request.user_data_ptr = &ud;
    request.user_data_size = sizeof(ud);

    request.buffer_ptr = self->buffer;
    request.buffer_size = request.chunk_size;
    
    self->handle = sfetch_send(&request);
    return 1;
}

static int l_dowork(lua_State* const L) {
    (void)L;
    sfetch_dowork();
    return 0;
}

LUAMOD_API int luaopen_sokol_fetch(lua_State* const L) {
    static luaL_Reg const functions[] = {
        {"setup", l_setup},
        {"shutdown", l_shutdown},
        {"valid", l_valid},
        {"desc", l_desc},
        {"max_path", l_max_path},
        {"send", l_send},
        {"dowork", l_dowork},

        {NULL, NULL}
    };

    luaL_newlib(L, functions);

    static lutil_IntConst const error_consts[] = {
        {"NO_ERROR", SFETCH_ERROR_NO_ERROR},
        {"FILE_NOT_FOUND", SFETCH_ERROR_FILE_NOT_FOUND},
        {"NO_BUFFER", SFETCH_ERROR_NO_BUFFER},
        {"BUFFER_TOO_SMALL", SFETCH_ERROR_BUFFER_TOO_SMALL},
        {"UNEXPECTED_EOF", SFETCH_ERROR_UNEXPECTED_EOF},
        {"INVALID_HTTP_STATUS", SFETCH_ERROR_INVALID_HTTP_STATUS},
        {"CANCELLED", SFETCH_ERROR_CANCELLED}
    };

    lutil_regintconsts(L, "error", error_consts, sizeof(error_consts) / sizeof(error_consts[0]));

    // Load the script that enriches the module
    if (luaL_loadbufferx(L, fetch_lua, sizeof(fetch_lua) / sizeof(fetch_lua[0]), "fetch.lua", "t") != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif
    }

    // Call the compiled chunk, it returns the function that will enrich the module
    if (lutil_pcall(L, 0, 1) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif
    }

    // Push the module and call the script
    lua_pushvalue(L, -2);

    if (lutil_pcall(L, 1, 0) != LUA_OK) {
#ifndef NDEBUG
        fprintf(stderr, "%s:%u: %s\n", __FILE__, __LINE__, lua_tostring(L, -1));
#else
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
#endif
    }

    return 1;
}
