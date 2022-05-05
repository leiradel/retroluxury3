#ifndef LUTIL_H__
#define LUTIL_H__

#include "vector.h"

#include <lua.h>

typedef struct {
    char const* name;
    lua_Integer value;
}
lutil_IntConst;

typedef struct {
    char const* const metatable_name;
    size_t const element_size;
    void (* const init_element)(lua_State* const L, void* const element, int const ndx);
    int (* const push_element_fields)(lua_State* const L, void const* const element);
}
lutil_VectorDesc;

int lutil_pcall(lua_State* const L, int const nargs, int const nres);
void lutil_regintconsts(lua_State* const L, char const* const name, lutil_IntConst const* const consts, size_t const count);

Vector* lutil_check_array(lutil_VectorDesc const* const desc, lua_State* const L, int const ndx);
Vector* lutil_push_array(lutil_VectorDesc const* const desc, lua_State* const L);

#endif /* LUTIL_H__ */
