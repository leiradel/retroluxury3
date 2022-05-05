#define _POSIX_C_SOURCE 199309L
#include <time.h> // for clock_gettime

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include <sokol_app.h>
#include <sokol_fetch.h>
#include <sokol_gfx.h>
#include <sokol_gp.h>
#include <sokol_glue.h>
