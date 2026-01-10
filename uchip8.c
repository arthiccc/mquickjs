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
#include <unistd.h>
#include <termios.h>

#include "cutils.h"
#include "mquickjs.h"

#define WIDTH 64
#define HEIGHT 32
static uint8_t vram[WIDTH * HEIGHT];

static struct termios old_tty;
static void restore_term(void) {
    printf("\x1b[?25h"); 
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tty);
}

static void setup_term(void) {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    old_tty = tty;
    atexit(restore_term);
    tty.c_lflag &= ~(ICANON | ECHO);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    printf("\x1b[?25l");
}

static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int i;
    for(i = 0; i < argc; i++) {
        if (i != 0) putchar(' ');
        if (JS_IsString(ctx, argv[i])) {
            JSCStringBuf buf;
            size_t len;
            const char *str = JS_ToCStringLen(ctx, &len, argv[i], &buf);
            fwrite(str, 1, len, stdout);
        } else {
            JS_PrintValueF(ctx, argv[i], 0);
        }
    }
    putchar('\n');
    return JS_UNDEFINED;
}

static JSValue js_draw(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int x, y, val;
    JS_ToInt32(ctx, &x, argv[0]);
    JS_ToInt32(ctx, &y, argv[1]);
    JS_ToInt32(ctx, &val, argv[2]);
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) vram[y * WIDTH + x] = val;
    return JS_UNDEFINED;
}

static JSValue js_gauntlet_flush(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    printf("\x1b[H");
    for (int y = 0; y < HEIGHT; y += 2) {
        for (int x = 0; x < WIDTH; x++) {
            int t = vram[y * WIDTH + x];
            int b = vram[(y + 1) * WIDTH + x];
            if (t && b) printf("█"); else if (t) printf("▀"); else if (b) printf("▄"); else printf(" ");
        }
        putchar('\n');
    }
    fflush(stdout);
    return JS_UNDEFINED;
}

static JSValue js_gauntlet_poll(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    uint8_t c;
    if (read(STDIN_FILENO, &c, 1) > 0) return JS_NewInt32(ctx, c);
    return JS_NewInt32(ctx, 0);
}

static JSValue js_get_rom(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf b;
    const char *path = JS_ToCString(ctx, argv[0], &b);
    FILE *f = fopen(path, "rb");
    if (!f) return JS_NULL;
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    JSGCRef arr_ref;
    JSValue *arr = JS_PushGCRef(ctx, &arr_ref);
    *arr = JS_NewArray(ctx, size);
    for (int i = 0; i < size; i++) JS_SetPropertyUint32(ctx, *arr, i, JS_NewInt32(ctx, fgetc(f)));
    fclose(f);
    return JS_PopGCRef(ctx, &arr_ref);
}

static int64_t get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000);
}

static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_NewInt64(ctx, get_time_ms()); }
static JSValue js_date_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_NewInt64(ctx, get_time_ms()); }
static JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { JS_GC(ctx); return JS_UNDEFINED; }
static JSValue js_load(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_setTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_clearTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_gauntlet_set(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }

#include "gauntlet_pro_stdlib.h"

int main(int argc, char **argv) {
    if (argc < 3) { printf("usage: uchip8 emu.js rom.ch8\n"); return 1; }
    size_t mem_size = 32 * 1024;
    uint8_t *mem_buf = malloc(mem_size);
    setup_term();
    JSContext *ctx = JS_NewContext(mem_buf, mem_size, &gauntlet_pro_stdlib);
    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "ROM_PATH", JS_NewString(ctx, argv[2]));
    
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
        restore_term();
        JS_PrintValueF(ctx, JS_GetException(ctx), JS_DUMP_LONG);
        printf("\n");
    }
    return 0;
}
