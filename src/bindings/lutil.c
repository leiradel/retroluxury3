#include "lutil.h"

#include <lauxlib.h>

#include <string.h>

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

typedef enum {
    STRUCTURE,
    SUBSTRUCTURE
}
NativeType;

typedef struct {
    lutil_CachedObject cached;
    NativeType type;
}
Native;

typedef struct {
    Native native;
    // structure data follows at offset NATIVE_HEADER_SIZE
}
Structure;

typedef struct {
    Native native;
    void* data;     // pointer to field in the parent structure
    int parent_ref; // reference to parent structure so it doesn't get collected
}
SubStructure;

#define NATIVE_HEADER_SIZE ((sizeof(Native) + 15) & ~15)

static void* get_data(Native* const native) {
    switch (native->type) {
        case STRUCTURE:
            return (uint8_t*)native + NATIVE_HEADER_SIZE;

        case SUBSTRUCTURE:
            return ((SubStructure*)native)->data;
    }

    return NULL;
}

static void push_substruct(void* const, int const, lutil_StructDesc* const, lua_State* const);

void* lutil_check_struct(lutil_StructDesc const* const desc, lua_State* const L, int const ndx) {
    void* const native = luaL_checkudata(L, ndx, desc->id);
    return get_data(native);
}

static int struct_index(lua_State* const L) {
    lutil_StructDesc* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Native* const native = luaL_checkudata(L, 1, desc->id);

    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    size_t const position = hash % desc->lookup_size;
    uint8_t const index = desc->lookup[position];

    if (index != 0) {
        lutil_FieldDesc const* const field = &desc->fields[index - 1];

        if (field->hash == hash && strcmp(field->id, key) == 0) {
            uint8_t* const struct_data = get_data(native);
            void* const field_data = struct_data + field->offset;

            switch (field->type) {
                case LUTIL_U64:
                    lua_pushinteger(L, *(uint64_t const*)field_data);
                    return 1;

                case LUTIL_U32:
                    lua_pushinteger(L, *(uint32_t const*)field_data);
                    return 1;

                case LUTIL_BOOL:
                    lua_pushboolean(L, *(bool const*)field_data);
                    return 1;

                case LUTIL_FLOAT:
                    lua_pushnumber(L, *(float const*)field_data);
                    return 1;

                case LUTIL_INT:
                case LUTIL_ENUM:
                    lua_pushinteger(L, *(int const*)field_data);
                    return 1;

                case LUTIL_UPTR:
                    lua_pushinteger(L, *(uintptr_t const*)field_data);
                    return 1;

                case LUTIL_STRUCT:
                    push_substruct(field_data, 1, field->structure, L);
                    return 1;
            }

            return luaL_error(L, "field %s type in %s is unknown", key, desc->id);
        }
    }

    return luaL_error(L, "unknown field %s in %s", key, desc->id);
}

static int struct_newindex(lua_State* const L) {
    lutil_StructDesc* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Native* const native = luaL_checkudata(L, 1, desc->id);

    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);
    size_t const position = hash % desc->lookup_size;
    uint8_t const index = desc->lookup[position];

    if (index != 0) {
        lutil_FieldDesc const* const field = &desc->fields[index - 1];

        if (field->hash == hash && strcmp(field->id, key) == 0) {
            uint8_t* const struct_data = get_data(native);
            void* const field_data = struct_data + field->offset;

            switch (field->type) {
                case LUTIL_U64:
                    *(uint64_t*)field_data = luaL_checkinteger(L, 3);
                    return 0;

                case LUTIL_U32:
                    *(uint32_t*)field_data = luaL_checkinteger(L, 3);
                    return 0;

                case LUTIL_BOOL:
                    *(bool*)field_data = lua_toboolean(L, 3);
                    return 0;

                case LUTIL_FLOAT:
                    *(float*)field_data = luaL_checknumber(L, 3);
                    return 0;

                case LUTIL_INT:
                case LUTIL_ENUM:
                    *(int*)field_data = luaL_checkinteger(L, 3);
                    return 0;

                case LUTIL_STRUCT:
                    break;

                case LUTIL_UPTR:
                    *(uintptr_t*)field_data = luaL_checkinteger(L, 3);
                    return 0;
            }

            return luaL_error(L, "field %s type in %s is unknown", key, desc->id);
        }
    }

    return luaL_error(L, "unknown field %s in %s", key, desc->id);
}

static int struct_gc(lua_State* const L) {
    lutil_StructDesc* const desc = lua_touserdata(L, lua_upvalueindex(1));
    Native* const native = luaL_checkudata(L, 1, desc->id);

    if (native->type == SUBSTRUCTURE) {
        luaL_unref(L, LUA_REGISTRYINDEX, ((SubStructure*)native)->parent_ref);
    }

    lutil_CachedObject* const cached = &native->cached;
    lutil_collect_cached(cached, L, &desc->free_list);

    return 0;
}

void* lutil_push_struct(lutil_StructDesc* const desc, lua_State* const L) {
    bool setmt = false;

    Structure* const structure = (Structure*)lutil_push_cached(
        L,
        &desc->free_list,
        NATIVE_HEADER_SIZE + desc->size,
        &setmt
    );

    structure->native.type = STRUCTURE;
    void* const data = get_data((Native*)structure);
    memset(data, 0, desc->size);

    if (setmt) {
        if (luaL_newmetatable(L, desc->id) != 0) {
            lua_pushlightuserdata(L, desc);
            lua_pushcclosure(L, struct_index, 1);
            lua_setfield(L, -2, "__index");

            lua_pushlightuserdata(L, desc);
            lua_pushcclosure(L, struct_newindex, 1);
            lua_setfield(L, -2, "__newindex");

            lua_pushlightuserdata(L, desc);
            lua_pushcclosure(L, struct_gc, 1);
            lua_setfield(L, -2, "__gc");
        }

        lua_setmetatable(L, -2);
    }

    return data;
}

static void push_substruct(void* const field_data, int const ndx, lutil_StructDesc* const desc, lua_State* const L) {
    static lutil_StructDesc substructures = {"substructure", 0, 0, NULL, 0, NULL, NULL};

    bool setmt = false;
    SubStructure* const substructure = (SubStructure*)lutil_push_cached(L, &substructures.free_list, sizeof(SubStructure), &setmt);
    substructure->native.type = SUBSTRUCTURE;

    lua_pushvalue(L, ndx);
    substructure->parent_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    substructure->data = field_data;

    if (setmt) {
        if (luaL_newmetatable(L, desc->id) != 0) {
            lua_pushlightuserdata(L, desc);
            lua_pushcclosure(L, struct_index, 1);
            lua_setfield(L, -2, "__index");

            lua_pushlightuserdata(L, desc);
            lua_pushcclosure(L, struct_newindex, 1);
            lua_setfield(L, -2, "__newindex");

            lua_pushlightuserdata(L, &substructures);
            lua_pushcclosure(L, struct_gc, 1);
            lua_setfield(L, -2, "__gc");
        }

        lua_setmetatable(L, -2);
    }
}
