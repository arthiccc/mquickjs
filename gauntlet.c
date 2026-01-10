#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <fcntl.h>

#include "cutils.h"
#include "mquickjs.h"

static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv)
{
    int i;
    for(i = 0; i < argc; i++) {
        if (i != 0) putchar(' ');
        if (JS_IsString(ctx, argv[i])) {
            JSCStringBuf buf;
            const char *str;
            size_t len;
            str = JS_ToCStringLen(ctx, &len, argv[i], &buf);
            fwrite(str, 1, len, stdout);
        } else {
            JS_PrintValueF(ctx, argv[i], 0);
        }
    }
    putchar('\n');
    return JS_UNDEFINED;
}

static JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv)
{
    JS_GC(ctx);
    return JS_UNDEFINED;
}

#if defined(__linux__) || defined(__APPLE__)
static int64_t get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000);
}
#else
static int64_t get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
}
#endif

static JSValue js_date_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv)
{
    return JS_NewInt64(ctx, get_time_ms());
}

static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv)
{
    return JS_NewInt64(ctx, get_time_ms());
}

static JSValue js_load(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_setTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_clearTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }

#include "mqjs_stdlib.h"

static void js_log_func(void *opaque, const void *buf, size_t buf_len)
{
    fwrite(buf, 1, buf_len, stdout);
}

int main(int argc, char **argv)
{
    size_t mem_size = 32 * 1024; 
    uint8_t *mem_buf;
    JSContext *ctx;
    JSValue global_obj, val;
    int buf_len;
    uint8_t *buf;
    const char *filename;

    if (argc < 2) {
        printf("usage: gauntlet script.js [mem_limit_kb]\n");
        return 1;
    }
    filename = argv[1];
    if (argc >= 3) {
        mem_size = atoi(argv[2]) * 1024;
    }

    mem_buf = malloc(mem_size);
    ctx = JS_NewContext(mem_buf, mem_size, &js_stdlib);
    JS_SetLogFunc(ctx, js_log_func);

    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror(filename);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    buf_len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc(buf_len + 1);
    fread(buf, 1, buf_len, f);
    buf[buf_len] = '\0';
    fclose(f);

    printf("\033[1;33m--- µGauntlet Initializing (%zu bytes RAM) ---\033[0m\n", mem_size);
    
    val = JS_Eval(ctx, (const char *)buf, buf_len, filename, 0);
    if (JS_IsException(val)) {
        JSValue ex = JS_GetException(ctx);
        printf("\033[1;31m[CRITICAL FAILURE]\033[0m ");
        JS_PrintValueF(ctx, ex, JS_DUMP_LONG);
        printf("\n");
        return 1;
    }

    // Check if it's a challenge mode (looking for a function named 'solution')
    global_obj = JS_GetGlobalObject(ctx);
    JSValue solution = JS_GetPropertyStr(ctx, global_obj, "solution");
    if (JS_IsFunction(ctx, solution)) {
        printf("\033[1;36m[CHALLENGE] Running 'solution' function...\033[0m\n");
        // We could run test cases here
        // JS_PushArg(ctx, JS_NewString(ctx, "aaabbbccc"));
        // JS_PushArg(ctx, solution);
        // JS_PushArg(ctx, JS_NULL);
        // val = JS_Call(ctx, 1);
        // ...
    }

    JS_FreeContext(ctx);
    free(mem_buf);
    free(buf);
    return 0;
}
