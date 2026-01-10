#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mquickjs.h"
#include "mquickjs_build.h"

// Prototypes for gauntlet_pro
static JSValue js_gauntlet_set(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_gauntlet_flush(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_gauntlet_poll(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

// Include standard library logic
#define main mqjs_main
#include "mqjs_stdlib.c"
#undef main

static const JSPropDef gauntlet_pro_global_object[] = {
    JS_PROP_CLASS_DEF("Object", &js_object_class),
    JS_PROP_CLASS_DEF("Function", &js_function_class),
    JS_PROP_CLASS_DEF("Number", &js_number_class),
    JS_PROP_CLASS_DEF("Boolean", &js_boolean_class),
    JS_PROP_CLASS_DEF("String", &js_string_class),
    JS_PROP_CLASS_DEF("Array", &js_array_class),
    JS_PROP_CLASS_DEF("Math", &js_math_obj),
    
    // Gauntlet Pro Bindings
    JS_CFUNC_DEF("set", 3, js_gauntlet_set),
    JS_CFUNC_DEF("flush", 0, js_gauntlet_flush),
    JS_CFUNC_DEF("poll", 0, js_gauntlet_poll),
    JS_CFUNC_DEF("performance_now", 0, js_performance_now),
    
    JS_PROP_END,
};

int main(int argc, char **argv)
{
    return build_atoms("gauntlet_pro_stdlib", gauntlet_pro_global_object, js_c_function_decl, argc, argv);
}
