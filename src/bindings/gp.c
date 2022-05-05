#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include <sokol_gfx.h>
#include <sokol_gp.h>
#include <sokol_glue.h>

#include "djb2.h"
#include "vector.h"
#include "lutil.h"

/*
 ######   ######   ########          ########  ########  ######  ########       ###    ########  ########     ###    ##    ## 
##    ## ##    ##  ##     ##         ##     ## ##       ##    ##    ##         ## ##   ##     ## ##     ##   ## ##    ##  ##  
##       ##        ##     ##         ##     ## ##       ##          ##        ##   ##  ##     ## ##     ##  ##   ##    ####   
 ######  ##   #### ########          ########  ######   ##          ##       ##     ## ########  ########  ##     ##    ##    
      ## ##    ##  ##                ##   ##   ##       ##          ##       ######### ##   ##   ##   ##   #########    ##    
##    ## ##    ##  ##                ##    ##  ##       ##    ##    ##       ##     ## ##    ##  ##    ##  ##     ##    ##    
 ######   ######   ##        ####### ##     ## ########  ######     ##       ##     ## ##     ## ##     ## ##     ##    ##    
*/

static void init_rect(lua_State* const L, void* const element, int const ndx) {
    sgp_rect* const self = element;

    self->x = luaL_checknumber(L, ndx);
    self->y = luaL_checknumber(L, ndx + 1);
    self->w = luaL_checknumber(L, ndx + 2);
    self->h = luaL_checknumber(L, ndx + 3);
}

static int push_rect_fields(lua_State* const L, void const* const element) {
    sgp_rect const* const self = element;

    lua_pushnumber(L, self->x);
    lua_pushnumber(L, self->y);
    lua_pushnumber(L, self->w);
    lua_pushnumber(L, self->h);

    return 4;
}

static lutil_VectorDesc RectArray = {
    "sgp_rect[]",
    sizeof(sgp_rect),
    init_rect,
    push_rect_fields
};

static int l_new_rect_array(lua_State* const L) {
    lutil_push_array(&RectArray, L);
    return 1;
}

/*
 ######   ######   ########          ######## ######## ##     ## ######## ##     ## ########  ######## ########          ########  ########  ######  ########       ###    ########  ########     ###    ##    ## 
##    ## ##    ##  ##     ##            ##    ##        ##   ##     ##    ##     ## ##     ## ##       ##     ##         ##     ## ##       ##    ##    ##         ## ##   ##     ## ##     ##   ## ##    ##  ##  
##       ##        ##     ##            ##    ##         ## ##      ##    ##     ## ##     ## ##       ##     ##         ##     ## ##       ##          ##        ##   ##  ##     ## ##     ##  ##   ##    ####   
 ######  ##   #### ########             ##    ######      ###       ##    ##     ## ########  ######   ##     ##         ########  ######   ##          ##       ##     ## ########  ########  ##     ##    ##    
      ## ##    ##  ##                   ##    ##         ## ##      ##    ##     ## ##   ##   ##       ##     ##         ##   ##   ##       ##          ##       ######### ##   ##   ##   ##   #########    ##    
##    ## ##    ##  ##                   ##    ##        ##   ##     ##    ##     ## ##    ##  ##       ##     ##         ##    ##  ##       ##    ##    ##       ##     ## ##    ##  ##    ##  ##     ##    ##    
 ######   ######   ##        #######    ##    ######## ##     ##    ##     #######  ##     ## ######## ########  ####### ##     ## ########  ######     ##       ##     ## ##     ## ##     ## ##     ##    ##    
*/

static void init_textured_rect(lua_State* const L, void* const element, int const ndx) {
    sgp_textured_rect* const self = element;

    self->dst.x = luaL_checknumber(L, ndx);
    self->dst.y = luaL_checknumber(L, ndx + 1);
    self->dst.w = luaL_checknumber(L, ndx + 2);
    self->dst.h = luaL_checknumber(L, ndx + 3);

    self->src.x = luaL_checknumber(L, ndx + 4);
    self->src.y = luaL_checknumber(L, ndx + 5);
    self->src.w = luaL_checknumber(L, ndx + 6);
    self->src.h = luaL_checknumber(L, ndx + 7);
}

static int push_textured_rect_fields(lua_State* const L, void const* const element) {
    sgp_textured_rect const* const self = element;

    lua_pushnumber(L, self->dst.x);
    lua_pushnumber(L, self->dst.y);
    lua_pushnumber(L, self->dst.w);
    lua_pushnumber(L, self->dst.h);

    lua_pushnumber(L, self->src.x);
    lua_pushnumber(L, self->src.y);
    lua_pushnumber(L, self->src.w);
    lua_pushnumber(L, self->src.h);

    return 8;
}

static lutil_VectorDesc TexturedRectArray = {
    "sgp_textured_rect[]",
    sizeof(sgp_textured_rect),
    init_textured_rect,
    push_textured_rect_fields
};

static int l_new_textured_rect_array(lua_State* const L) {
    lutil_push_array(&TexturedRectArray, L);
    return 1;
}

/*
 ######   ######   ########          ########   #######  #### ##    ## ########       ###    ########  ########     ###    ##    ## 
##    ## ##    ##  ##     ##         ##     ## ##     ##  ##  ###   ##    ##         ## ##   ##     ## ##     ##   ## ##    ##  ##  
##       ##        ##     ##         ##     ## ##     ##  ##  ####  ##    ##        ##   ##  ##     ## ##     ##  ##   ##    ####   
 ######  ##   #### ########          ########  ##     ##  ##  ## ## ##    ##       ##     ## ########  ########  ##     ##    ##    
      ## ##    ##  ##                ##        ##     ##  ##  ##  ####    ##       ######### ##   ##   ##   ##   #########    ##    
##    ## ##    ##  ##                ##        ##     ##  ##  ##   ###    ##       ##     ## ##    ##  ##    ##  ##     ##    ##    
 ######   ######   ##        ####### ##         #######  #### ##    ##    ##       ##     ## ##     ## ##     ## ##     ##    ##    
*/

static void init_point(lua_State* const L, void* const element, int const ndx) {
    sgp_point* const self = element;

    self->x = luaL_checknumber(L, ndx);
    self->y = luaL_checknumber(L, ndx + 1);
}

static int push_point_fields(lua_State* const L, void const* const element) {
    sgp_point const* const self = element;

    lua_pushnumber(L, self->x);
    lua_pushnumber(L, self->y);

    return 2;
}

static lutil_VectorDesc PointArray = {
    "sgp_point[]",
    sizeof(sgp_point),
    init_point,
    push_point_fields
};

static int l_new_point_array(lua_State* const L) {
    lutil_push_array(&PointArray, L);
    return 1;
}

/*
 ######   ######   ########          ##       #### ##    ## ########       ###    ########  ########     ###    ##    ## 
##    ## ##    ##  ##     ##         ##        ##  ###   ## ##            ## ##   ##     ## ##     ##   ## ##    ##  ##  
##       ##        ##     ##         ##        ##  ####  ## ##           ##   ##  ##     ## ##     ##  ##   ##    ####   
 ######  ##   #### ########          ##        ##  ## ## ## ######      ##     ## ########  ########  ##     ##    ##    
      ## ##    ##  ##                ##        ##  ##  #### ##          ######### ##   ##   ##   ##   #########    ##    
##    ## ##    ##  ##                ##        ##  ##   ### ##          ##     ## ##    ##  ##    ##  ##     ##    ##    
 ######   ######   ##        ####### ######## #### ##    ## ########    ##     ## ##     ## ##     ## ##     ##    ##    
*/

static void init_line(lua_State* const L, void* const element, int const ndx) {
    sgp_line* const self = element;

    self->a.x = luaL_checknumber(L, ndx);
    self->a.y = luaL_checknumber(L, ndx + 1);
    self->b.x = luaL_checknumber(L, ndx + 2);
    self->b.y = luaL_checknumber(L, ndx + 3);
}

static int push_line_fields(lua_State* const L, void const* const element) {
    sgp_line const* const self = element;

    lua_pushnumber(L, self->a.x);
    lua_pushnumber(L, self->a.y);
    lua_pushnumber(L, self->b.x);
    lua_pushnumber(L, self->b.y);

    return 4;
}

static lutil_VectorDesc LineArray = {
    "sgp_line[]",
    sizeof(sgp_line),
    init_line,
    push_line_fields
};

static int l_new_line_array(lua_State* const L) {
    lutil_push_array(&LineArray, L);
    return 1;
}

/*
 ######   ######   ########          ######## ########  ####    ###    ##    ##  ######   ##       ########       ###    ########  ########     ###    ##    ## 
##    ## ##    ##  ##     ##            ##    ##     ##  ##    ## ##   ###   ## ##    ##  ##       ##            ## ##   ##     ## ##     ##   ## ##    ##  ##  
##       ##        ##     ##            ##    ##     ##  ##   ##   ##  ####  ## ##        ##       ##           ##   ##  ##     ## ##     ##  ##   ##    ####   
 ######  ##   #### ########             ##    ########   ##  ##     ## ## ## ## ##   #### ##       ######      ##     ## ########  ########  ##     ##    ##    
      ## ##    ##  ##                   ##    ##   ##    ##  ######### ##  #### ##    ##  ##       ##          ######### ##   ##   ##   ##   #########    ##    
##    ## ##    ##  ##                   ##    ##    ##   ##  ##     ## ##   ### ##    ##  ##       ##          ##     ## ##    ##  ##    ##  ##     ##    ##    
 ######   ######   ##        #######    ##    ##     ## #### ##     ## ##    ##  ######   ######## ########    ##     ## ##     ## ##     ## ##     ##    ##    
*/

static void init_triangle(lua_State* const L, void* const element, int const ndx) {
    sgp_triangle* const self = element;

    self->a.x = luaL_checknumber(L, ndx);
    self->a.y = luaL_checknumber(L, ndx + 1);
    self->b.x = luaL_checknumber(L, ndx + 2);
    self->b.y = luaL_checknumber(L, ndx + 3);
    self->c.x = luaL_checknumber(L, ndx + 2);
    self->c.y = luaL_checknumber(L, ndx + 3);
}

static int push_triangle_fields(lua_State* const L, void const* const element) {
    sgp_triangle const* const self = element;

    lua_pushnumber(L, self->a.x);
    lua_pushnumber(L, self->a.y);
    lua_pushnumber(L, self->b.x);
    lua_pushnumber(L, self->b.y);
    lua_pushnumber(L, self->c.x);
    lua_pushnumber(L, self->c.y);

    return 6;
}

static lutil_VectorDesc TriangleArray = {
    "sgp_triangle[]",
    sizeof(sgp_line),
    init_triangle,
    push_triangle_fields
};

static int l_new_triangle_array(lua_State* const L) {
    lutil_push_array(&TriangleArray, L);
    return 1;
}

/*
 ######   ######   ########          ########  ########  ######   ######  
##    ## ##    ##  ##     ##         ##     ## ##       ##    ## ##    ## 
##       ##        ##     ##         ##     ## ##       ##       ##       
 ######  ##   #### ########          ##     ## ######    ######  ##       
      ## ##    ##  ##                ##     ## ##             ## ##       
##    ## ##    ##  ##                ##     ## ##       ##    ## ##    ## 
 ######   ######   ##        ####### ########  ########  ######   ######  
*/

#define SGP_DESC_MT "sgp_desc"

typedef struct {
    sgp_desc desc;
}
Desc;

static Desc* desc_check(lua_State* const L, int const ndx) {
    return luaL_checkudata(L, ndx, SGP_DESC_MT);
}

static int desc_index(lua_State* const L) {
    Desc const* const self = desc_check(L, 1);
    char const* const key = luaL_checkstring(L, 2);
    djb2_hash const hash = djb2(key);

    switch (hash) {
        case DJB2HASH_C(0x2ef0bbaf): /* max_vertices */
            lua_pushinteger(L, self->desc.max_vertices);
            return 1;

        case DJB2HASH_C(0x9ba7a37c): /* max_commands */
            lua_pushinteger(L, self->desc.max_commands);
            return 1;
    }

    return luaL_error(L, "unknown field %s in %s", key, SGP_DESC_MT);
}

static Desc* desc_push(lua_State* const L) {
    Desc* const self = lua_newuserdata(L, sizeof(*self));

    if (luaL_newmetatable(L, SGP_DESC_MT) != 0) {
        lua_pushcfunction(L, desc_index);
        lua_setfield(L, -2, "__index");
    }

    lua_setmetatable(L, -2);
    return self;
}

/*
 ######   #######  ##    ##  #######  ##                ######   ########  
##    ## ##     ## ##   ##  ##     ## ##               ##    ##  ##     ## 
##       ##     ## ##  ##   ##     ## ##               ##        ##     ## 
 ######  ##     ## #####    ##     ## ##               ##   #### ########  
      ## ##     ## ##  ##   ##     ## ##               ##    ##  ##        
##    ## ##     ## ##   ##  ##     ## ##               ##    ##  ##        
 ######   #######  ##    ##  #######  ######## #######  ######   ##        
*/

static int l_setup(lua_State* const L) {
    sgp_desc desc;
    memset(&desc, 0, sizeof(desc));

    if (!lua_isnone(L, 1)) {
        if (lua_getfield(L, 1, "max_vertices") != LUA_TNIL) {
            desc.max_vertices = luaL_checkinteger(L, -1);
        }

        if (lua_getfield(L, 1, "max_commands") != LUA_TNIL) {
            desc.max_commands = luaL_checkinteger(L, -1);
        }
    }

    sgp_setup(&desc);
    return 0;
}

static int l_shutdown(lua_State* const L) {
    (void)L;
    sgp_shutdown();
    return 0;
}

static int l_is_valid(lua_State* const L) {
    bool const isvalid = sgp_is_valid();
    lua_pushboolean(L, isvalid);
    return 1;
}

static int l_get_last_error(lua_State* const L) {
    sgp_error const error = sgp_get_last_error();
    lua_pushinteger(L, error);
    return 1;
}

static int l_get_error_message(lua_State* const L) {
    sgp_error const error = luaL_checkinteger(L, 1);
    char const* const message = sgp_get_error_message(error);
    lua_pushstring(L, message);
    return 1;
}

// TODO sgp_make_pipeline

static int l_begin(lua_State* const L) {
    int const width = luaL_checkinteger(L, 1);
    int const height = luaL_checkinteger(L, 2);

    sgp_begin(width, height);
    return 0;
}

static int l_flush(lua_State* const L) {
    (void)L;
    sgp_flush();
    return 0;
}

static int l_end(lua_State* const L) {
    (void)L;
    sgp_end();
    return 0;
}

static int l_project(lua_State* const L) {
    float const left = luaL_checknumber(L, 1);
    float const right = luaL_checknumber(L, 2);
    float const top = luaL_checknumber(L, 3);
    float const bottom = luaL_checknumber(L, 4);

    sgp_project(left, right, top, bottom);
    return 0;
}

static int l_reset_project(lua_State* const L) {
    (void)L;
    sgp_reset_project();
    return 0;
}

static int l_push_transform(lua_State* const L) {
    (void)L;
    sgp_push_transform();
    return 0;
}

static int l_pop_transform(lua_State* const L) {
    (void)L;
    sgp_pop_transform();
    return 0;
}

static int l_reset_transform(lua_State* const L) {
    (void)L;
    sgp_reset_transform();
    return 0;
}

static int l_translate(lua_State* const L) {
    float const x = luaL_checknumber(L, 1);
    float const y = luaL_checknumber(L, 2);

    sgp_translate(x, y);
    return 0;
}

static int l_rotate(lua_State* const L) {
    float const theta = luaL_checknumber(L, 1);
    sgp_rotate(theta);
    return 0;
}

static int l_rotate_at(lua_State* const L) {
    float const theta = luaL_checknumber(L, 1);
    float const x = luaL_checknumber(L, 2);
    float const y = luaL_checknumber(L, 3);

    sgp_rotate_at(theta, x, y);
    return 0;
}

static int l_scale(lua_State* const L) {
    float const sx = luaL_checknumber(L, 1);
    float const sy = luaL_checknumber(L, 2);

    sgp_scale(sx, sy);
    return 0;
}

static int l_scale_at(lua_State* const L) {
    float const sx = luaL_checknumber(L, 1);
    float const sy = luaL_checknumber(L, 2);
    float const x = luaL_checknumber(L, 3);
    float const y = luaL_checknumber(L, 4);

    sgp_scale_at(sx, sy, x, y);
    return 0;
}

// TODO sgp_set_pipeline

static int l_reset_pipeline(lua_State* const L) {
    (void)L;
    sgp_reset_pipeline();
    return 0;
}

// TODO sgp_set_uniform

static int l_reset_uniform(lua_State* const L) {
    (void)L;
    sgp_reset_uniform();
    return 0;
}

static int l_set_blend_mode(lua_State* const L) {
    sgp_blend_mode const blend_mode = luaL_checkinteger(L, 1);
    sgp_set_blend_mode(blend_mode);
    return 0;
}

static int l_reset_blend_mode(lua_State* const L) {
    (void)L;
    sgp_reset_blend_mode();
    return 0;
}

static int l_set_color(lua_State* const L) {
    float const r = luaL_checknumber(L, 1);
    float const g = luaL_checknumber(L, 2);
    float const b = luaL_checknumber(L, 3);
    float const a = luaL_checknumber(L, 4);

    sgp_set_color(r, g, b, a);
    return 0;
}

static int l_reset_color(lua_State* const L) {
    (void)L;
    sgp_reset_color();
    return 0;
}

// TODO sgp_set_image

static int l_unset_image(lua_State* const L) {
    int const channel = luaL_checkinteger(L, 1);
    sgp_unset_image(channel);
    return 0;
}

static int l_reset_image(lua_State* const L) {
    int const channel = luaL_checkinteger(L, 1);
    sgp_reset_image(channel);
    return 0;
}

static int l_viewport(lua_State* const L) {
    int const x = luaL_checkinteger(L, 1);
    int const y = luaL_checkinteger(L, 2);
    int const w = luaL_checkinteger(L, 3);
    int const h = luaL_checkinteger(L, 4);

    sgp_viewport(x, y, w, h);
    return 0;
}

static int l_reset_viewport(lua_State* const L) {
    (void)L;
    sgp_reset_viewport();
    return 0;
}

static int l_scissor(lua_State* const L) {
    int const x = luaL_checkinteger(L, 1);
    int const y = luaL_checkinteger(L, 2);
    int const w = luaL_checkinteger(L, 3);
    int const h = luaL_checkinteger(L, 4);

    sgp_scissor(x, y, w, h);
    return 0;
}

static int l_reset_scissor(lua_State* const L) {
    (void)L;
    sgp_reset_scissor();
    return 0;
}

static int l_reset_state(lua_State* const L) {
    (void)L;
    sgp_reset_state();
    return 0;
}

static int l_clear(lua_State* const L) {
    (void)L;
    sgp_clear();
    return 0;
}

static int l_draw_points(lua_State* const L) {
    Vector* const vector = lutil_check_array(&PointArray, L, 1);
    sgp_draw_points(vector->elements, vector->count);
    return 0;
}

static int l_draw_point(lua_State* const L) {
    float const x = luaL_checknumber(L, 1);
    float const y = luaL_checknumber(L, 2);

    sgp_draw_point(x, y);
    return 0;
}

static int l_draw_lines(lua_State* const L) {
    Vector* const vector = lutil_check_array(&LineArray, L, 1);
    sgp_draw_lines(vector->elements, vector->count);
    return 0;
}

static int l_draw_line(lua_State* const L) {
    float const ax = luaL_checknumber(L, 1);
    float const ay = luaL_checknumber(L, 2);
    float const bx = luaL_checknumber(L, 3);
    float const by = luaL_checknumber(L, 4);

    sgp_draw_line(ax, ay, bx, by);
    return 0;
}

static int l_draw_lines_strip(lua_State* const L) {
    Vector* const vector = lutil_check_array(&PointArray, L, 1);
    sgp_draw_lines_strip(vector->elements, vector->count);
    return 0;
}

static int l_draw_filled_triangles(lua_State* const L) {
    Vector* const vector = lutil_check_array(&TriangleArray, L, 1);
    sgp_draw_filled_triangles(vector->elements, vector->count);
    return 0;
}

static int l_draw_filled_triangle(lua_State* const L) {
    float const ax = luaL_checknumber(L, 1);
    float const ay = luaL_checknumber(L, 2);
    float const bx = luaL_checknumber(L, 3);
    float const by = luaL_checknumber(L, 4);
    float const cx = luaL_checknumber(L, 5);
    float const cy = luaL_checknumber(L, 6);

    sgp_draw_filled_triangle(ax, ay, bx, by, cx, cy);
    return 0;
}

static int l_draw_filled_triangles_strip(lua_State* const L) {
    Vector* const vector = lutil_check_array(&PointArray, L, 1);
    sgp_draw_filled_triangles_strip(vector->elements, vector->count);
    return 0;
}

static int l_draw_filled_rects(lua_State* const L) {
    Vector* const vector = lutil_check_array(&RectArray, L, 1);
    sgp_draw_filled_rects(vector->elements, vector->count);
    return 0;
}

static int l_draw_filled_rect(lua_State* const L) {
    float const x = luaL_checknumber(L, 1);
    float const y = luaL_checknumber(L, 2);
    float const w = luaL_checknumber(L, 3);
    float const h = luaL_checknumber(L, 4);

    sgp_draw_filled_rect(x, y, w, h);
    return 0;
}

static int l_draw_textured_rects(lua_State* const L) {
    Vector* const vector = lutil_check_array(&RectArray, L, 1);
    sgp_draw_textured_rects(vector->elements, vector->count);
    return 0;
}

static int l_draw_textured_rect(lua_State* const L) {
    float const x = luaL_checknumber(L, 1);
    float const y = luaL_checknumber(L, 2);
    float const w = luaL_checknumber(L, 3);
    float const h = luaL_checknumber(L, 4);

    sgp_draw_textured_rect(x, y, w, h);
    return 0;
}

static int l_draw_textured_rects_ex(lua_State* const L) {
    int const channel = luaL_checkinteger(L, 1);
    Vector* const vector = lutil_check_array(&RectArray, L, 2);

    sgp_draw_textured_rects_ex(channel, vector->elements, vector->count);
    return 0;
}

static int l_draw_textured_rect_ex(lua_State* const L) {
    int const channel = luaL_checkinteger(L, 1);

    sgp_rect const dest_rect = {
        luaL_checknumber(L, 2),
        luaL_checknumber(L, 3),
        luaL_checknumber(L, 4),
        luaL_checknumber(L, 5)
    };

    sgp_rect const src_rect = {
        luaL_checknumber(L, 6),
        luaL_checknumber(L, 7),
        luaL_checknumber(L, 8),
        luaL_checknumber(L, 9)
    };

    sgp_draw_textured_rect_ex(channel, dest_rect, src_rect);
    return 0;
}

// TODO sgp_query_state

static int l_query_desc(lua_State* const L) {
    sgp_desc const desc = sgp_query_desc();
    desc_push(L)->desc = desc;
    return 1;
}

/*
##       ##     ##    ###     #######  ########  ######## ##    ##          ######   #######  ##    ##  #######  ##                ######   ########  
##       ##     ##   ## ##   ##     ## ##     ## ##       ###   ##         ##    ## ##     ## ##   ##  ##     ## ##               ##    ##  ##     ## 
##       ##     ##  ##   ##  ##     ## ##     ## ##       ####  ##         ##       ##     ## ##  ##   ##     ## ##               ##        ##     ## 
##       ##     ## ##     ## ##     ## ########  ######   ## ## ##          ######  ##     ## #####    ##     ## ##               ##   #### ########  
##       ##     ## ######### ##     ## ##        ##       ##  ####               ## ##     ## ##  ##   ##     ## ##               ##    ##  ##        
##       ##     ## ##     ## ##     ## ##        ##       ##   ###         ##    ## ##     ## ##   ##  ##     ## ##               ##    ##  ##        
########  #######  ##     ##  #######  ##        ######## ##    ## #######  ######   #######  ##    ##  #######  ######## #######  ######   ##        
*/

LUAMOD_API int luaopen_sokol_gp(lua_State* const L) {
    static luaL_Reg const functions[] = {
        {"new_rect_array", l_new_rect_array},
        {"new_textured_rect_array", l_new_textured_rect_array},
        {"new_point_array", l_new_point_array},
        {"new_line_array", l_new_line_array},
        {"new_triangle_array", l_new_triangle_array},
        {"setup", l_setup},
        {"shutdown", l_shutdown},
        {"is_valid", l_is_valid},
        {"get_last_error", l_get_last_error},
        {"get_error_message", l_get_error_message},
        {"begin", l_begin},
        {"flush", l_flush},
        {"end_", l_end},
        {"project", l_project},
        {"reset_project", l_reset_project},
        {"push_transform", l_push_transform},
        {"pop_transform", l_pop_transform},
        {"reset_transform", l_reset_transform},
        {"translate", l_translate},
        {"rotate", l_rotate},
        {"rotate_at", l_rotate_at},
        {"scale", l_scale},
        {"scale_at", l_scale_at},
        {"reset_pipeline", l_reset_pipeline},
        {"reset_uniform", l_reset_uniform},
        {"set_blend_mode", l_set_blend_mode},
        {"reset_blend_mode", l_reset_blend_mode},
        {"set_color", l_set_color},
        {"reset_color", l_reset_color},
        {"unset_image", l_unset_image},
        {"reset_image", l_reset_image},
        {"viewport", l_viewport},
        {"reset_viewport", l_reset_viewport},
        {"scissor", l_scissor},
        {"reset_scissor", l_reset_scissor},
        {"reset_state", l_reset_state},
        {"clear", l_clear},
        {"draw_points", l_draw_points},
        {"draw_point", l_draw_point},
        {"draw_lines", l_draw_lines},
        {"draw_line", l_draw_line},
        {"draw_lines_strip", l_draw_lines_strip},
        {"draw_filled_triangles", l_draw_filled_triangles},
        {"draw_filled_triangle", l_draw_filled_triangle},
        {"draw_filled_triangles_strip", l_draw_filled_triangles_strip},
        {"draw_filled_rects", l_draw_filled_rects},
        {"draw_filled_rect", l_draw_filled_rect},
        {"draw_textured_rects", l_draw_textured_rects},
        {"draw_textured_rect", l_draw_textured_rect},
        {"draw_textured_rects_ex", l_draw_textured_rects_ex},
        {"draw_textured_rect_ex", l_draw_textured_rect_ex},
        {"query_desc", l_query_desc},

        {NULL, NULL}
    };

    luaL_newlib(L, functions);

    static lutil_IntConst const error_consts[] = {
        {"NO_ERROR", SGP_NO_ERROR},
        {"SOKOL_INVALID", SGP_ERROR_SOKOL_INVALID},
        {"VERTICES_FULL", SGP_ERROR_VERTICES_FULL},
        {"UNIFORMS_FULL", SGP_ERROR_UNIFORMS_FULL},
        {"COMMANDS_FULL", SGP_ERROR_COMMANDS_FULL},
        {"VERTICES_OVERFLOW", SGP_ERROR_VERTICES_OVERFLOW},
        {"TRANSFORM_STACK_OVERFLOW", SGP_ERROR_TRANSFORM_STACK_OVERFLOW},
        {"TRANSFORM_STACK_UNDERFLOW", SGP_ERROR_TRANSFORM_STACK_UNDERFLOW},
        {"STATE_STACK_OVERFLOW", SGP_ERROR_STATE_STACK_OVERFLOW},
        {"STATE_STACK_UNDERFLOW", SGP_ERROR_STATE_STACK_UNDERFLOW},
        {"ALLOC_FAILED", SGP_ERROR_ALLOC_FAILED},
        {"MAKE_VERTEX_BUFFER_FAILED", SGP_ERROR_MAKE_VERTEX_BUFFER_FAILED},
        {"MAKE_WHITE_IMAGE_FAILED", SGP_ERROR_MAKE_WHITE_IMAGE_FAILED},
        {"MAKE_COMMON_SHADER_FAILED", SGP_ERROR_MAKE_COMMON_SHADER_FAILED},
        {"MAKE_COMMON_PIPELINE_FAILED", SGP_ERROR_MAKE_COMMON_PIPELINE_FAILED}
    };

    lutil_regintconsts(L, "error", error_consts, sizeof(error_consts) / sizeof(error_consts[0]));

    static lutil_IntConst const blendmode_consts[] = {
        {"NONE", SGP_BLENDMODE_NONE},
        {"BLEND", SGP_BLENDMODE_BLEND},
        {"ADD", SGP_BLENDMODE_ADD},
        {"MOD", SGP_BLENDMODE_MOD},
        {"MUL", SGP_BLENDMODE_MUL}
    };

    lutil_regintconsts(L, "blendmode", blendmode_consts, sizeof(blendmode_consts) / sizeof(blendmode_consts[0]));

    return 1;
}
