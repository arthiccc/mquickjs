#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mquickjs.h"
#include "mquickjs_build.h"

static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);
static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv);

static const JSPropDef minimal_global_object[] = {
    JS_CFUNC_DEF("print", 1, js_print),
    JS_CFUNC_DEF("now", 0, js_performance_now),
    JS_PROP_END,
};

int main(int argc, char **argv) {
    return build_atoms("ubrain_stdlib", minimal_global_object, NULL, argc, argv);
}
