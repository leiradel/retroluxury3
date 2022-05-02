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
