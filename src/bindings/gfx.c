#include "gfx.h"

#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include <sokol_gfx.h>
#include <sokol_app.h>
#include <sokol_glue.h>

#include "djb2.h"
#include "lutil.h"

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

static void* free_descs = NULL;

SgDesc* sg_desc_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SG_DESC_MT);
}

static int sg_desc_index(lua_State* const L) {
    SgDesc const* const self = sg_desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x42d88132): /* buffer_pool_size */
            lua_pushinteger(L, self->data.buffer_pool_size);
            return 1;

        case DJB2HASH_C(0x789879bb): /* image_pool_size */
            lua_pushinteger(L, self->data.image_pool_size);
            return 1;

        case DJB2HASH_C(0x00c03f2f): /* shader_pool_size */
            lua_pushinteger(L, self->data.shader_pool_size);
            return 1;

        case DJB2HASH_C(0x03a9b7ee): /* pipeline_pool_size */
            lua_pushinteger(L, self->data.pipeline_pool_size);
            return 1;

        case DJB2HASH_C(0x3bc833ef): /* pass_pool_size */
            lua_pushinteger(L, self->data.pass_pool_size);
            return 1;

        case DJB2HASH_C(0x89372a5d): /* context_pool_size */
            lua_pushinteger(L, self->data.context_pool_size);
            return 1;

        case DJB2HASH_C(0x73889538): /* uniform_buffer_size */
            lua_pushinteger(L, self->data.uniform_buffer_size);
            return 1;

        case DJB2HASH_C(0x7ed6f945): /* staging_buffer_size */
            lua_pushinteger(L, self->data.staging_buffer_size);
            return 1;

        case DJB2HASH_C(0x7b524926): /* sampler_cache_size */
            lua_pushinteger(L, self->data.sampler_cache_size);
            return 1;

        case DJB2HASH_C(0xd3799aca): /* context */
            sg_context_desc_push(L)->data = self->data.context;
            return 1;
    }

    return luaL_error(L, "unknown field %s in %s", key, SG_DESC_MT);
}

static int sg_desc_newindex(lua_State* const L) {
    SgDesc* const self = sg_desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x42d88132): /* buffer_pool_size */
            self->data.buffer_pool_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x789879bb): /* image_pool_size */
            self->data.image_pool_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x00c03f2f): /* shader_pool_size */
            self->data.shader_pool_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x03a9b7ee): /* pipeline_pool_size */
            self->data.pipeline_pool_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x3bc833ef): /* pass_pool_size */
            self->data.pass_pool_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x89372a5d): /* context_pool_size */
            self->data.context_pool_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x73889538): /* uniform_buffer_size */
            self->data.uniform_buffer_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x7ed6f945): /* staging_buffer_size */
            self->data.staging_buffer_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x7b524926): /* sampler_cache_size */
            self->data.sampler_cache_size = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0xd3799aca): /* context */
            self->data.context = sg_context_desc_check(L, 3)->data;
            return 0;
    }

    return luaL_error(L, "unknown field %s in %s", key, SG_DESC_MT);
}

static int sg_desc_gc(lua_State* const L) {
    SgDesc* const self = sg_desc_check(L, 1);
    lutil_collect_cached((lutil_CachedObject*)self, L, &free_descs);
    return 0;
}

SgDesc* sg_desc_push(lua_State* const L) {
    bool setmt = false;
    SgDesc* self = (SgDesc*)lutil_push_cached(L, &free_descs, sizeof(*self), &setmt);

    if (setmt) {
        if (luaL_newmetatable(L, SG_DESC_MT) != 0) {
            lua_pushcfunction(L, sg_desc_index);
            lua_setfield(L, -2, "__index");

            lua_pushcfunction(L, sg_desc_newindex);
            lua_setfield(L, -2, "__newindex");

            lua_pushcfunction(L, sg_desc_gc);
            lua_setfield(L, -2, "__gc");
        }

        lua_setmetatable(L, -2);
    }

    return self;
}

static int l_new_desc(lua_State* const L) {
    SgDesc* const self = sg_desc_push(L);
    memset(&self->data, 0, sizeof(self->data));
    return 1;
}

/*
 ######   ######            ######   #######  ##    ## ######## ######## ##     ## ########         ########  ########  ######   ######  
##    ## ##    ##          ##    ## ##     ## ###   ##    ##    ##        ##   ##     ##            ##     ## ##       ##    ## ##    ## 
##       ##                ##       ##     ## ####  ##    ##    ##         ## ##      ##            ##     ## ##       ##       ##       
 ######  ##   ####         ##       ##     ## ## ## ##    ##    ######      ###       ##            ##     ## ######    ######  ##       
      ## ##    ##          ##       ##     ## ##  ####    ##    ##         ## ##      ##            ##     ## ##             ## ##       
##    ## ##    ##          ##    ## ##     ## ##   ###    ##    ##        ##   ##     ##            ##     ## ##       ##    ## ##    ## 
 ######   ######   #######  ######   #######  ##    ##    ##    ######## ##     ##    ##    ####### ########  ########  ######   ######  
*/

/*
These fields are not really a good fit for application written in Lua, and were
left out of this binding:

* sg_gl_context_desc gl;
* sg_metal_context_desc metal;
* sg_d3d11_context_desc d3d11;
* sg_wgpu_context_desc wgpu;
*/

#define SG_CONTEXT_DESC_MT "sg_context_desc"

static void* free_context_descs = NULL;

SgContextDesc* sg_context_desc_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SG_CONTEXT_DESC_MT);
}

static int sg_context_desc_index(lua_State* const L) {
    SgContextDesc const* const self = sg_context_desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x0c91394c): /* color_format */
            lua_pushinteger(L, self->data.color_format);
            return 1;

        case DJB2HASH_C(0x23ebaae2): /* depth_format */
            lua_pushinteger(L, self->data.depth_format);
            return 1;

        case DJB2HASH_C(0x7c5fd34f): /* sample_count */
            lua_pushinteger(L, self->data.sample_count);
            return 1;
    }

    return luaL_error(L, "unknown field %s in %s", key, SG_CONTEXT_DESC_MT);
}

static int sg_context_desc_newindex(lua_State* const L) {
    SgContextDesc* const self = sg_context_desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x0c91394c): /* color_format */
            self->data.color_format = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x23ebaae2): /* depth_format */
            self->data.depth_format = luaL_checkinteger(L, 3);
            return 0;

        case DJB2HASH_C(0x7c5fd34f): /* sample_count */
            self->data.sample_count = luaL_checkinteger(L, 3);
            return 0;
    }

    return luaL_error(L, "unknown field %s in %s", key, SG_CONTEXT_DESC_MT);
}

static int sg_context_desc_gc(lua_State* const L) {
    SgContextDesc* const self = sg_context_desc_check(L, 1);
    lutil_collect_cached((lutil_CachedObject*)self, L, &free_context_descs);
    return 0;
}

SgContextDesc* sg_context_desc_push(lua_State* const L) {
    bool setmt = false;
    SgContextDesc* self = (SgContextDesc*)lutil_push_cached(L, &free_descs, sizeof(*self), &setmt);

    if (setmt) {
        if (luaL_newmetatable(L, SG_CONTEXT_DESC_MT) != 0) {
            lua_pushcfunction(L, sg_context_desc_index);
            lua_setfield(L, -2, "__index");

            lua_pushcfunction(L, sg_context_desc_newindex);
            lua_setfield(L, -2, "__newindex");

            lua_pushcfunction(L, sg_context_desc_gc);
            lua_setfield(L, -2, "__gc");
        }

        lua_setmetatable(L, -2);
    }

    return self;
}

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

typedef struct {
    lutil_CachedObject cached;
    sg_pass_action pass_action;
}
PassAction;

static void* free_pass_actions = NULL;

static PassAction* pass_action_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SG_PASS_ACTION_MT);
}

static int pass_action_gc(lua_State* const L) {
    PassAction* const self = pass_action_check(L, 1);
    lutil_collect_cached((lutil_CachedObject*)self, L, &free_pass_actions);
    return 0;
}

static int pass_action_push(lua_State* const L, int const ndx) {
    bool const has_desc = !lua_isnone(L, ndx);

    bool setmt = false;
    PassAction* const self = (PassAction*)lutil_push_cached(L, &free_pass_actions, sizeof(PassAction), &setmt);
    memset(&self->pass_action, 0, sizeof(self->pass_action));

    if (has_desc) {
        int const top = lua_gettop(L);

        if (lua_getfield(L, ndx, "colors") != LUA_TNIL) {
            for (lua_Integer i = 1; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
                lua_geti(L, -1, i);

                if (lua_isnil(L, -1)) {
                    lua_pop(L, 1);
                    break;
                }

                lua_geti(L, -1, 1);
                self->pass_action.colors[i - 1].action = luaL_checkinteger(L, -1);

                lua_geti(L, -2, 2);
                self->pass_action.colors[i - 1].value.r = luaL_checknumber(L, -1);

                lua_geti(L, -3, 3);
                self->pass_action.colors[i - 1].value.g = luaL_checknumber(L, -1);

                lua_geti(L, -4, 4);
                self->pass_action.colors[i - 1].value.b = luaL_checknumber(L, -1);

                lua_geti(L, -5, 5);
                self->pass_action.colors[i - 1].value.a = luaL_checknumber(L, -1);

                lua_pop(L, 6);
            }
        }

        if (lua_getfield(L, ndx, "depth") != LUA_TNIL) {
            if (lua_rawgeti(L, -1, 1) != LUA_TNIL) {
                self->pass_action.depth.action = luaL_checkinteger(L, -1);
            }

            if (lua_rawgeti(L, -2, 2) != LUA_TNIL) {
                self->pass_action.depth.value = luaL_checknumber(L, -1);
            }
        }

        if (lua_getfield(L, ndx, "stencil") != LUA_TNIL) {
            if (lua_rawgeti(L, -1, 1) != LUA_TNIL) {
                self->pass_action.stencil.action = luaL_checkinteger(L, -1);
            }

            if (lua_rawgeti(L, -2, 2) != LUA_TNIL) {
                self->pass_action.stencil.value = luaL_checkinteger(L, -1);
            }
        }

        lua_settop(L, top);
    }

    if (setmt) {
        if (luaL_newmetatable(L, SG_PASS_ACTION_MT) != 0) {
            lua_pushcfunction(L, pass_action_gc);
            lua_setfield(L, -2, "__gc");
        }

        lua_setmetatable(L, -2);
    }

    return 1;
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
    SgDesc const* const desc = sg_desc_check(L, 1);
    sg_setup(&desc->data);
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
    PassAction const* const pass_action = pass_action_check(L, 1);
    int const width = luaL_checkinteger(L, 2);
    int const height = luaL_checkinteger(L, 3);

    sg_begin_default_pass(&pass_action->pass_action, width, height);
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
        {"new_desc", l_new_desc},
        {"new_pass_action", l_new_pass_action},
        {"setup", l_setup},
        {"shutdown", l_shutdown},
        {"isvalid", l_isvalid},
        {"begin_default_pass", l_begin_default_pass},
        {"end_pass", l_end_pass},
        {"commit", l_commit},

        {NULL, NULL}
    };

    luaL_newlib(L, functions);

    return 1;
}
