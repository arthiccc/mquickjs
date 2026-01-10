#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#include "cutils.h"
#include "mquickjs.h"

// Implementation of the C functions exposed to JS
static char *read_file_to_str(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t n = fread(buf, 1, len, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

static JSValue js_readFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *path = JS_ToCString(ctx, argv[0], &buf);
    if (!path) return JS_EXCEPTION;
    char *content = read_file_to_str(path);
    if (!content) return JS_NULL;
    JSValue res = JS_NewString(ctx, content);
    free(content);
    return res;
}

static JSValue js_writeFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf b1, b2;
    const char *path = JS_ToCString(ctx, argv[0], &b1);
    if (!path) return JS_EXCEPTION;
    size_t len;
    const char *content = JS_ToCStringLen(ctx, &len, argv[1], &b2);
    if (!content) return JS_EXCEPTION;
    FILE *f = fopen(path, "wb");
    if (!f) return JS_FALSE;
    fwrite(content, 1, len, f);
    fclose(f);
    return JS_TRUE;
}

static JSValue js_listFiles(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf buf;
    const char *path = JS_ToCString(ctx, argv[0], &buf);
    if (!path) return JS_EXCEPTION;
    DIR *d = opendir(path);
    if (!d) return JS_NULL;
    
    JSGCRef arr_ref;
    JSValue *arr = JS_PushGCRef(ctx, &arr_ref);
    *arr = JS_NewArray(ctx, 0);
    
    struct dirent *dir;
    int i = 0;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;
        JS_SetPropertyUint32(ctx, *arr, i++, JS_NewString(ctx, dir->d_name));
    }
    closedir(d);
    return JS_PopGCRef(ctx, &arr_ref);
}

static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    for(int i = 0; i < argc; i++) {
        if (i != 0) printf(" ");
        JSCStringBuf buf;
        const char *str = JS_ToCString(ctx, argv[i], &buf);
        if (str) printf("%s", str);
    }
    printf("\n");
    return JS_UNDEFINED;
}

static JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JS_GC(ctx);
    return JS_UNDEFINED;
}

#include "ustatic_stdlib.h"

int main(int argc, char **argv) {
    printf("DEBUG: Starting ustatic...\n");
    if (argc < 2) {
        printf("usage: ustatic ssg.js\n");
        return 1;
    }
    
    size_t mem_size = 1024 * 1024; // 1MB for the SSG
    uint8_t *mem_buf = malloc(mem_size);
    if (!mem_buf) return 1;
    
    JSContext *ctx = JS_NewContext(mem_buf, mem_size, &ustatic_stdlib);
    
    char *script = read_file_to_str(argv[1]);
    if (!script) {
        fprintf(stderr, "Could not read script %s\n", argv[1]);
        return 1;
    }
    
    JSValue val = JS_Eval(ctx, script, strlen(script), argv[1], 0);
    if (JS_IsException(val)) {
        JSValue ex = JS_GetException(ctx);
        printf("EXCEPTION: ");
        JS_PrintValueF(ctx, ex, JS_DUMP_LONG);
        printf("\n");
    }
    
    JS_FreeContext(ctx);
    free(script);
    free(mem_buf);
    return 0;
}
