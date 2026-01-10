#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "mquickjs.h"
#include "mquickjs_build.h"

// Prototypes for the build utility
static JSValue js_readFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_writeFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_listFiles(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

// Redefine main to avoid conflict when including mqjs_stdlib.c
#define main mqjs_main
#include "mqjs_stdlib.c"
#undef main

static const JSPropDef ustatic_global_object_full[] = {
    // Standard JS objects (from mqjs_stdlib.c)
    JS_PROP_CLASS_DEF("Object", &js_object_class),
    JS_PROP_CLASS_DEF("Function", &js_function_class),
    JS_PROP_CLASS_DEF("Number", &js_number_class),
    JS_PROP_CLASS_DEF("Boolean", &js_boolean_class),
    JS_PROP_CLASS_DEF("String", &js_string_class),
    JS_PROP_CLASS_DEF("Array", &js_array_class),
    JS_PROP_CLASS_DEF("Math", &js_math_obj),
    JS_PROP_CLASS_DEF("JSON", &js_json_obj),
    
    // Our new functions
    JS_CFUNC_DEF("readFile", 1, js_readFile),
    JS_CFUNC_DEF("writeFile", 2, js_writeFile),
    JS_CFUNC_DEF("listFiles", 1, js_listFiles),
    JS_CFUNC_DEF("print", 1, js_print),
    JS_CFUNC_DEF("gc", 0, js_gc),
    
    JS_PROP_END,
};

int main(int argc, char **argv)
{
    return build_atoms("ustatic_stdlib", ustatic_global_object_full, js_c_function_decl, argc, argv);
}
