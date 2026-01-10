#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mquickjs.h"

static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    for(int i = 0; i < argc; i++) {
        JSCStringBuf buf;
        size_t len;
        const char *str = JS_ToCStringLen(ctx, &len, argv[i], &buf);
        if (str) fwrite(str, 1, len, stdout);
    }
    return JS_UNDEFINED;
}

static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return JS_NewInt64(ctx, (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000));
}

// Required stubs for standard library
static JSValue js_date_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_NewInt32(ctx, 0); }
static JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { JS_GC(ctx); return JS_UNDEFINED; }
static JSValue js_load(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_setTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_clearTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }

#include "mqjs_stdlib.h"

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    size_t mem_size = 16 * 1024; // THE 16KB WALL
    uint8_t *mem_buf = malloc(mem_size);
    
    JSContext *ctx = JS_NewContext(mem_buf, mem_size, &js_stdlib);
    if (!ctx) return 1;
    
    FILE *f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *src = malloc(len + 1);
    fread(src, 1, len, f);
    src[len] = '\0';
    fclose(f);
    
    JSValue val = JS_Eval(ctx, src, len, argv[1], 0);
    if (JS_IsException(val)) {
        JS_PrintValueF(ctx, JS_GetException(ctx), JS_DUMP_LONG);
        printf("\n");
    }
    return 0;
}
