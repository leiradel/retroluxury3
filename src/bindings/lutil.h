#ifndef LUTIL_H__
#define LUTIL_H__

#include "vector.h"
#include "djb2.h"

#include <lua.h>
#include <stdbool.h>

typedef struct {
    char const* name;
    lua_Integer value;
}
lutil_IntConst;

int lutil_pcall(lua_State* const L, int const nargs, int const nres);
void lutil_regintconsts(lua_State* const L, char const* const name, lutil_IntConst const* const consts, size_t const count);

typedef struct {
    char const* const metatable_name;
    size_t const element_size;
    void (* const init_element)(lua_State* const L, void* const element, int const ndx);
    int (* const push_element_fields)(lua_State* const L, void const* const element);
}
lutil_VectorDesc;

Vector* lutil_check_array(lutil_VectorDesc const* const desc, lua_State* const L, int const ndx);
Vector* lutil_push_array(lutil_VectorDesc const* const desc, lua_State* const L);

typedef struct {
    void* next;
    int ref;
}
lutil_CachedObject;

lutil_CachedObject* lutil_push_cached(lua_State* const L, void** list_head, size_t const size, bool* const setmt);
void lutil_collect_cached(lutil_CachedObject* const cached, lua_State* const L, void** list_head);

#define LUTIL_CACHED_SIZE ((sizeof(lutil_CachedObject) + 15) & ~15)
#define LUTIL_OFS(n, f) ((uint8_t*)&((n*)0x10)->f - (uint8_t*)0x10)
#define LUTIL_STRUCT(s, n) ((n*)(uint8_t*)s + LUTIL_CACHED_SIZE)

typedef enum {
    LUTIL_U64,
    LUTIL_U32,
    LUTIL_BOOL,
    LUTIL_FLOAT,
    LUTIL_INT,
    LUTIL_ENUM,
    LUTIL_STRUCT
}
lutil_Type;

typedef struct {
    djb2_hash const hash;
    lutil_Type type;
    const char* id;
    size_t const offset;
}
lutil_FieldDesc;

typedef struct {
    char const* const name;
    size_t const size;
    size_t const num_fields;
    lutil_FieldDesc const* const fields;
    uint16_t const lookup_size;
    uint8_t const* const lookup;
    void* free_list;
}
lutil_StructDesc;

void* lutil_check_struct(lutil_StructDesc const* const desc, lua_State* const L, int const ndx);
void* lutil_push_struct(lutil_StructDesc* const desc, lua_State* const L);

#endif /* LUTIL_H__ */
