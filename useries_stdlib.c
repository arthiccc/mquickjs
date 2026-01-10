#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "mquickjs.h"
#include "mquickjs_build.h"

// Unified Prototypes
static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_gauntlet_poll(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_gauntlet_flush(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_gauntlet_set(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_draw(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_get_rom(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_readFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_writeFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_listFiles(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

// Include standard library logic
#define main mqjs_main
#include "mqjs_stdlib.c"
#undef main

static const JSPropDef useries_global_object[] = {
    JS_PROP_CLASS_DEF("Object", &js_object_class),
    JS_PROP_CLASS_DEF("Function", &js_function_class),
    JS_PROP_CLASS_DEF("Number", &js_number_class),
    JS_PROP_CLASS_DEF("Boolean", &js_boolean_class),
    JS_PROP_CLASS_DEF("String", &js_string_class),
    JS_PROP_CLASS_DEF("Array", &js_array_class),
    JS_PROP_CLASS_DEF("Uint8Array", &js_Uint8Array_class),
    JS_PROP_CLASS_DEF("Math", &js_math_obj),
    JS_PROP_CLASS_DEF("JSON", &js_json_obj),
    
    // Core
    JS_CFUNC_DEF("print", 1, js_print),
    JS_CFUNC_DEF("now", 0, js_performance_now),
    
    // TTY & Graphics
    JS_CFUNC_DEF("poll", 0, js_gauntlet_poll),
    JS_CFUNC_DEF("flush", 0, js_gauntlet_flush),
    JS_CFUNC_DEF("set", 3, js_gauntlet_set),
    JS_CFUNC_DEF("draw", 3, js_draw),
    
    // Systems & Emulation
    JS_CFUNC_DEF("get_rom", 1, js_get_rom),
    JS_CFUNC_DEF("readFile", 1, js_readFile),
    JS_CFUNC_DEF("writeFile", 2, js_writeFile),
    JS_CFUNC_DEF("listFiles", 1, js_listFiles),
    
    JS_PROP_END,
};

int main(int argc, char **argv)
{
    return build_atoms("useries_stdlib", useries_global_object, js_c_function_decl, argc, argv);
}
