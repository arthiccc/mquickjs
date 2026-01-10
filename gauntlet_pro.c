#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>

#include "cutils.h"
#include "mquickjs.h"

#define WIDTH 80
#define HEIGHT 24
static uint8_t frame_buffer[WIDTH * HEIGHT];

static struct termios old_tty;
static void restore_term(void) {
    printf("\x1b[?25h"); // Show cursor
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
    printf("\x1b[?25l"); // Hide cursor
}

static char *read_file_to_str(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    size_t n = fread(buf, 1, len, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

static JSValue js_gauntlet_set(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int x, y, c;
    if (JS_ToInt32(ctx, &x, argv[0])) return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &y, argv[1])) return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &c, argv[2])) return JS_EXCEPTION;
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        frame_buffer[y * WIDTH + x] = (uint8_t)c;
    }
    return JS_UNDEFINED;
}

static JSValue js_gauntlet_flush(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    printf("\x1b[H"); // Home
    for (int y = 0; y < HEIGHT; y++) {
        fwrite(&frame_buffer[y * WIDTH], 1, WIDTH, stdout);
        if (y < HEIGHT - 1) putchar('\n');
    }
    fflush(stdout);
    usleep(33333); // Cap at ~30 FPS to avoid flooding terminal
    return JS_UNDEFINED;
}

static JSValue js_gauntlet_poll(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    uint8_t c;
    if (read(STDIN_FILENO, &c, 1) > 0) return JS_NewInt32(ctx, c);
    return JS_NewInt32(ctx, 0);
}

static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return JS_NewInt64(ctx, (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000));
}

// Minimal stubs for full stdlib compatibility if needed
static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { JS_GC(ctx); return JS_UNDEFINED; }
static JSValue js_date_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_NewInt32(ctx, 0); }

#include "gauntlet_pro_stdlib.h"

int main(int argc, char **argv) {
    if (argc < 2) { printf("usage: gauntlet_pro script.js\n"); return 1; }
    
    size_t mem_size = 16 * 1024; // THE HARD LIMIT
    uint8_t *mem_buf = malloc(mem_size);
    if (!mem_buf) return 1;
    
    setup_term();
    
    JSContext *ctx = JS_NewContext(mem_buf, mem_size, &gauntlet_pro_stdlib);
    
    char *script = read_file_to_str(argv[1]);
    if (!script) return 1;
    
    JSValue val = JS_Eval(ctx, script, strlen(script), argv[1], 0);
    if (JS_IsException(val)) {
        restore_term();
        JSValue ex = JS_GetException(ctx);
        printf("\x1b[31mCRASH: ");
        JS_PrintValueF(ctx, ex, JS_DUMP_LONG);
        printf("\x1b[0m\n");
    }
    
    JS_FreeContext(ctx);
    free(script);
    free(mem_buf);
    return 0;
}
