#ifndef LUTIL_H__
#define LUTIL_H__

#include <lua.h>

typedef struct {
    char const* name;
    lua_Integer value;
}
lutil_IntConst;

int lutil_pcall(lua_State* const L, int const nargs, int const nres);
void lutil_regintconsts(lua_State* const L, char const* const name, lutil_IntConst const* const consts, size_t const count);

#endif /* LUTIL_H__ */
