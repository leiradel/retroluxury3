#include "lutil.h"

#include <lauxlib.h>

static int traceback(lua_State* const L) {
    luaL_traceback(L, L, lua_tostring(L, -1), 1);
    return 1;
}

int lutil_pcall(lua_State* const L, int const nargs, int const nres) {
    int const errndx = lua_gettop(L) - nargs;
    lua_pushcfunction(L, traceback);
    lua_insert(L, errndx);
  
    int const ret = lua_pcall(L, nargs, nres, errndx);
    lua_remove(L, errndx);

    return ret;
}

void lutil_regintconsts(lua_State* const L, char const* const name, lutil_IntConst const* const consts, size_t const count) {
    if (name != NULL) {
        lua_createtable(L, count, 0);
    }

    for (size_t i = 0; i < count; i++) {
        lua_pushstring(L, consts[i].name);
        lua_pushinteger(L, consts[i].value);
        lua_rawset(L, -3);

        lua_pushstring(L, consts[i].name);
        lua_rawseti(L, -2, consts[i].value);
    }

    if (name != NULL) {
        lua_setfield(L, -2, name);
    }
}

static int array_set(lua_State* const L) {
    lutil_VectorDesc const* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Vector* const self = lutil_check_array(desc, L, 1);

    lua_Integer const ndx = luaL_checkinteger(L, 2);
    void* const element = vector_at(self, ndx - 1);

    if (element == NULL) {
        return luaL_error(L, "index %I out of bounds", ndx);
    }

    desc->init_element(L, element, 3);
    return 0;
}

static int array_get(lua_State* const L) {
    lutil_VectorDesc const* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Vector* const self = lutil_check_array(desc, L, 1);

    lua_Integer const ndx = luaL_checkinteger(L, 2);
    void const* const element = vector_at(self, ndx - 1);

    if (element == NULL) {
        return luaL_error(L, "index %I out of bounds", ndx);
    }

    return desc->push_element_fields(L, element);
}

static int array_append(lua_State* const L) {
    lutil_VectorDesc const* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Vector* const self = lutil_check_array(desc, L, 1);

    void* const element = vector_append(self);

    if (element == NULL) {
        return luaL_error(L, "out of memory");
    }

    desc->init_element(L, element, 2);
    return 0;
}

static int array_reserve(lua_State* const L) {
    lutil_VectorDesc const* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Vector* const self = lutil_check_array(desc, L, 1);

    lua_Integer const new_capacity = luaL_checkinteger(L, 2);

    if (vector_reserve(self, new_capacity) != 0) {
        return luaL_error(L, "out of memory");
    }

    return 0;
}

static int array_resize(lua_State* const L) {
    lutil_VectorDesc const* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Vector* const self = lutil_check_array(desc, L, 1);

    lua_Integer const new_count = luaL_checkinteger(L, 2);

    if (vector_resize(self, new_count) != 0) {
        return luaL_error(L, "out of memory");
    }

    return 0;
}

static int array_shrink(lua_State* const L) {
    lutil_VectorDesc const* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Vector* const self = lutil_check_array(desc, L, 1);
    vector_shrink(self);
    return 0;
}

static int array_len(lua_State* const L) {
    lutil_VectorDesc const* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Vector* const self = lutil_check_array(desc, L, 1);
    lua_pushinteger(L, self->count);
    return 1;
}

static int array_gc(lua_State* const L) {
    lutil_VectorDesc const* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Vector* const self = lutil_check_array(desc, L, 1);
    vector_destroy(self);
    return 0;
}

Vector* lutil_check_array(lutil_VectorDesc const* const desc, lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, desc->metatable_name);
}

Vector* lutil_push_array(lutil_VectorDesc const* const desc, lua_State* const L) {
    Vector* const self = lua_newuserdata(L, sizeof(Vector));
    vector_init(self, desc->element_size);

    if (luaL_newmetatable(L, desc->metatable_name) != 0) {
        static luaL_Reg const methods[] = {
            {"set", array_set},
            {"get", array_get},
            {"append", array_append},
            {"reserve", array_reserve},
            {"resize", array_resize},
            {"shrink", array_shrink},
            {NULL, NULL}
        };

        luaL_newlibtable(L, methods);
        lua_pushlightuserdata(L, (void*)desc);
        luaL_setfuncs(L, methods, 1);
        lua_setfield(L, -2, "__index");

        lua_pushlightuserdata(L, (void*)desc);
        lua_pushcclosure(L, array_len, 1);
        lua_setfield(L, -2, "__len");

        lua_pushlightuserdata(L, (void*)desc);
        lua_pushcclosure(L, array_gc, 1);
        lua_setfield(L, -2, "__gc");
    }

    lua_setmetatable(L, -2);
    return self;
}

lutil_CachedObject* lutil_push_cached(lua_State* const L, void** list_head, size_t const size, bool* const setmt) {
    lutil_CachedObject* cached = NULL;

    if (*list_head != NULL) {
        cached = *list_head;
        *list_head = cached->next;

        lua_rawgeti(L, LUA_REGISTRYINDEX, cached->ref);

        luaL_unref(L, LUA_REGISTRYINDEX, cached->ref);
        cached->ref = LUA_NOREF;

        *setmt = false;
    }
    else {
        cached = lua_newuserdata(L, size);
        cached->next = NULL;
        cached->ref = LUA_NOREF;

        *setmt = true;
    }

    return cached;
}

void lutil_collect_cached(lutil_CachedObject* const cached, lua_State* const L, void** list_head) {
    cached->next = *list_head;
    *list_head = cached;

    lua_pushvalue(L, 1);
    cached->ref = luaL_ref(L, LUA_REGISTRYINDEX);
}
