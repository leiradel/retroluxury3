#ifndef GFX_H__
#define GFX_H__

#include <sokol_gfx.h>

#include "lutil.h"

typedef struct {
    lutil_CachedObject cached;
    sg_desc data;
}
SgDesc;

SgDesc* sg_desc_check(lua_State* const L, int const ndx);
SgDesc* sg_desc_push(lua_State* const L);

typedef struct {
    lutil_CachedObject cached;
    sg_context_desc data;
}
SgContextDesc;

SgContextDesc* sg_context_desc_check(lua_State* const L, int const ndx);
SgContextDesc* sg_context_desc_push(lua_State* const L);

#endif /* GFX_H__ */
