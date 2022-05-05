#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include <sokol_gfx.h>
#include <sokol_app.h>
#include <sokol_glue.h>

#include "djb2.h"

/*
 ######   ######           ########  ########  ######   ######  
##    ## ##    ##          ##     ## ##       ##    ## ##    ## 
##       ##                ##     ## ##       ##       ##       
 ######  ##   ####         ##     ## ######    ######  ##       
      ## ##    ##          ##     ## ##             ## ##       
##    ## ##    ##          ##     ## ##       ##    ## ##    ## 
 ######   ######   ####### ########  ########  ######   ######  
*/

#define SG_DESC_MT "sg_desc"

typedef struct {
    sg_desc desc;
}
Desc;

static Desc* desc_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SG_DESC_MT);
}

#if 0
static int desc_index(lua_State* const L) {
    Desc const* const self = desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x42d88132): /* buffer_pool_size */
            lua_pushinteger(L, self->desc.buffer_pool_size);
            return 1;

        case DJB2HASH_C(0x789879bb): /* image_pool_size */
            lua_pushinteger(L, self->desc.image_pool_size);
            return 1;

        case DJB2HASH_C(0x00c03f2f): /* shader_pool_size */
            lua_pushinteger(L, self->desc.shader_pool_size);
            return 1;

        case DJB2HASH_C(0x03a9b7ee): /* pipeline_pool_size */
            lua_pushinteger(L, self->desc.pipeline_pool_size);
            return 1;

        case DJB2HASH_C(0x3bc833ef): /* pass_pool_size */
            lua_pushinteger(L, self->desc.pass_pool_size);
            return 1;

        case DJB2HASH_C(0x89372a5d): /* context_pool_size */
            lua_pushinteger(L, self->desc.context_pool_size);
            return 1;

        case DJB2HASH_C(0x73889538): /* uniform_buffer_size */
            lua_pushinteger(L, self->desc.uniform_buffer_size);
            return 1;

        case DJB2HASH_C(0x7ed6f945): /* staging_buffer_size */
            lua_pushinteger(L, self->desc.staging_buffer_size);
            return 1;

        case DJB2HASH_C(0x7b524926): /* sampler_cache_size */
            lua_pushinteger(L, self->desc.sampler_cache_size);
            return 1;
    }

    return luaL_error(L, "unknown field %s in %s", key, SG_DESC_MT);
}

#ifndef NDEBUG
static int desc_newindex(lua_State* const L) {
    desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x42d88132): /* buffer_pool_size */
        case DJB2HASH_C(0x789879bb): /* image_pool_size */
        case DJB2HASH_C(0x00c03f2f): /* shader_pool_size */
        case DJB2HASH_C(0x03a9b7ee): /* pipeline_pool_size */
        case DJB2HASH_C(0x3bc833ef): /* pass_pool_size */
        case DJB2HASH_C(0x89372a5d): /* context_pool_size */
        case DJB2HASH_C(0x73889538): /* uniform_buffer_size */
        case DJB2HASH_C(0x7ed6f945): /* staging_buffer_size */
        case DJB2HASH_C(0x7b524926): /* sampler_cache_size */
            return luaL_error(L, "cannot change field %s in %s, object is read-only", key, SG_DESC_MT);
    }

    return luaL_error(L, "unknown field %s in %s", key, SG_DESC_MT);
}
#endif

static Desc* desc_push(lua_State* const L) {
    Desc* const self = lua_newuserdata(L, sizeof(*self));

    if (luaL_newmetatable(L, SG_DESC_MT) != 0) {
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
#endif

/*
 ######   ######           ########     ###     ######   ######             ###     ######  ######## ####  #######  ##    ## 
##    ## ##    ##          ##     ##   ## ##   ##    ## ##    ##           ## ##   ##    ##    ##     ##  ##     ## ###   ## 
##       ##                ##     ##  ##   ##  ##       ##                ##   ##  ##          ##     ##  ##     ## ####  ## 
 ######  ##   ####         ########  ##     ##  ######   ######          ##     ## ##          ##     ##  ##     ## ## ## ## 
      ## ##    ##          ##        #########       ##       ##         ######### ##          ##     ##  ##     ## ##  #### 
##    ## ##    ##          ##        ##     ## ##    ## ##    ##         ##     ## ##    ##    ##     ##  ##     ## ##   ### 
 ######   ######   ####### ##        ##     ##  ######   ######  ####### ##     ##  ######     ##    ####  #######  ##    ## 
*/

#define SG_PASS_ACTION_MT "sg_pass_action"

static int pass_action_push(lua_State* const L, int const ndx) {
    sg_pass_action* const self = lua_newuserdata(L, sizeof(*self));
    memset(self, 0, sizeof(*self));

    int const top = lua_gettop(L);

    if (lua_getfield(L, ndx, "colors") != LUA_TNIL) {
        for (lua_Integer i = 1; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
            lua_geti(L, -1, i);

            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                break;
            }

            lua_geti(L, -1, 1);
            self->colors[i - 1].action = luaL_checkinteger(L, -1);

            lua_geti(L, -2, 2);
            self->colors[i - 1].value.r = luaL_checknumber(L, -1);

            lua_geti(L, -3, 3);
            self->colors[i - 1].value.g = luaL_checknumber(L, -1);

            lua_geti(L, -4, 4);
            self->colors[i - 1].value.b = luaL_checknumber(L, -1);

            lua_geti(L, -5, 5);
            self->colors[i - 1].value.a = luaL_checknumber(L, -1);

            lua_pop(L, 6);
        }
    }

    if (lua_getfield(L, ndx, "depth") != LUA_TNIL) {
        if (lua_rawgeti(L, -1, 1) != LUA_TNIL) {
            self->depth.action = luaL_checkinteger(L, -1);
        }

        if (lua_rawgeti(L, -2, 2) != LUA_TNIL) {
            self->depth.value = luaL_checknumber(L, -1);
        }
    }

    if (lua_getfield(L, ndx, "stencil") != LUA_TNIL) {
        if (lua_rawgeti(L, -1, 1) != LUA_TNIL) {
            self->stencil.action = luaL_checkinteger(L, -1);
        }

        if (lua_rawgeti(L, -2, 2) != LUA_TNIL) {
            self->stencil.value = luaL_checkinteger(L, -1);
        }
    }

    lua_settop(L, top);

    if (luaL_newmetatable(L, SG_PASS_ACTION_MT) != 0) {
        // TODO
    }

    lua_setmetatable(L, -2);
    return 1;
}

static sg_pass_action* pass_action_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SG_PASS_ACTION_MT);
}

static int l_new_pass_action(lua_State* const L) {
    return pass_action_push(L, 1);
}

/*
 ######   #######  ##    ##  #######  ##                ######   ######## ##     ## 
##    ## ##     ## ##   ##  ##     ## ##               ##    ##  ##        ##   ##  
##       ##     ## ##  ##   ##     ## ##               ##        ##         ## ##   
 ######  ##     ## #####    ##     ## ##               ##   #### ######      ###    
      ## ##     ## ##  ##   ##     ## ##               ##    ##  ##         ## ##   
##    ## ##     ## ##   ##  ##     ## ##               ##    ##  ##        ##   ##  
 ######   #######  ##    ##  #######  ######## #######  ######   ##       ##     ## 
*/

static int l_setup(lua_State* const L) {
    Desc const* const desc = desc_check(L, 1);
    sg_setup(&desc->desc);
    return 0;
}

static int l_shutdown(lua_State* const L) {
    (void)L;
    sg_shutdown();
    return 0;
}

static int l_isvalid(lua_State* const L) {
    bool const isvalid = sg_isvalid();
    lua_pushboolean(L, isvalid);
    return 1;
}

static int l_begin_default_pass(lua_State* const L) {
    sg_pass_action const* const pass_action = pass_action_check(L, 1);
    int const width = luaL_checkinteger(L, 2);
    int const height = luaL_checkinteger(L, 3);

    sg_begin_default_pass(pass_action, width, height);
    return 0;
}

static int l_end_pass(lua_State* const L) {
    (void)L;
    sg_end_pass();
    return 0;
}

static int l_commit(lua_State* const L) {
    (void)L;
    sg_commit();
    return 0;
}

// From sokol_glue
static int l_setup_from_app(lua_State* const L) {
    (void)L;

    sg_desc const desc = {
        .context = sapp_sgcontext()
    };

    sg_setup(&desc);
    return 0;
}

/*
##       ##     ##    ###     #######  ########  ######## ##    ##          ######   #######  ##    ##  #######  ##                ######   ######## ##     ## 
##       ##     ##   ## ##   ##     ## ##     ## ##       ###   ##         ##    ## ##     ## ##   ##  ##     ## ##               ##    ##  ##        ##   ##  
##       ##     ##  ##   ##  ##     ## ##     ## ##       ####  ##         ##       ##     ## ##  ##   ##     ## ##               ##        ##         ## ##   
##       ##     ## ##     ## ##     ## ########  ######   ## ## ##          ######  ##     ## #####    ##     ## ##               ##   #### ######      ###    
##       ##     ## ######### ##     ## ##        ##       ##  ####               ## ##     ## ##  ##   ##     ## ##               ##    ##  ##         ## ##   
##       ##     ## ##     ## ##     ## ##        ##       ##   ###         ##    ## ##     ## ##   ##  ##     ## ##               ##    ##  ##        ##   ##  
########  #######  ##     ##  #######  ##        ######## ##    ## #######  ######   #######  ##    ##  #######  ######## #######  ######   ##       ##     ## 
*/

LUAMOD_API int luaopen_sokol_gfx(lua_State* const L) {
    static luaL_Reg const functions[] = {
        {"new_pass_action", l_new_pass_action},
        {"setup", l_setup},
        {"shutdown", l_shutdown},
        {"isvalid", l_isvalid},
        {"begin_default_pass", l_begin_default_pass},
        {"end_pass", l_end_pass},
        {"commit", l_commit},
        {"setup_from_app", l_setup_from_app},

        {NULL, NULL}
    };

    luaL_newlib(L, functions);

    return 1;
}
