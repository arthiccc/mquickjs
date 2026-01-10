#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <dirent.h>

#include "cutils.h"
#include "mquickjs.h"

// --- SYSTEM STATE ---
#define WIDTH 80
#define HEIGHT 24
static uint8_t vram[WIDTH * HEIGHT];
static struct termios old_tty;
static int is_tty_raw = 0;

// --- TERMINAL UTILS ---
static void restore_term(void) {
    if (is_tty_raw) {
        printf("\x1b[?25h"); // Show cursor
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tty);
        is_tty_raw = 0;
    }
}

static void setup_term_raw(void) {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    old_tty = tty;
    tty.c_lflag &= ~(ICANON | ECHO);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    printf("\x1b[?25l"); // Hide cursor
    is_tty_raw = 1;
}

// --- JS BINDINGS IMPLEMENTATION ---
static JSValue js_print(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    for(int i = 0; i < argc; i++) {
        if (i != 0) putchar(' ');
        JSCStringBuf buf; size_t len;
        const char *str = JS_ToCStringLen(ctx, &len, argv[i], &buf);
        if (str) fwrite(str, 1, len, stdout);
    }
    putchar('\n');
    return JS_UNDEFINED;
}

static JSValue js_performance_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return JS_NewInt64(ctx, (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec / 1000000));
}

static JSValue js_gauntlet_poll(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    uint8_t c;
    if (read(STDIN_FILENO, &c, 1) > 0) return JS_NewInt32(ctx, c);
    return JS_NewInt32(ctx, 0);
}

static JSValue js_gauntlet_set(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int x, y, c;
    JS_ToInt32(ctx, &x, argv[0]); JS_ToInt32(ctx, &y, argv[1]); JS_ToInt32(ctx, &c, argv[2]);
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) vram[y * WIDTH + x] = (uint8_t)c;
    return JS_UNDEFINED;
}

static JSValue js_draw(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    int x, y, val;
    JS_ToInt32(ctx, &x, argv[0]); JS_ToInt32(ctx, &y, argv[1]); JS_ToInt32(ctx, &val, argv[2]);
    // Use center of screen for CHIP8 (64x32)
    int ox = (WIDTH - 64)/2, oy = (HEIGHT - 32)/2;
    if (x >= 0 && x < 64 && y >= 0 && y < 32) vram[(oy+y) * WIDTH + (ox+x)] = val ? '#' : ' ';
    return JS_UNDEFINED;
}

static JSValue js_gauntlet_flush(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    printf("\x1b[H");
    for (int y = 0; y < HEIGHT; y++) {
        fwrite(&vram[y * WIDTH], 1, WIDTH, stdout);
        if (y < HEIGHT - 1) putchar('\n');
    }
    fflush(stdout);
    usleep(16000);
    return JS_UNDEFINED;
}

static JSValue js_get_rom(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf b; const char *path = JS_ToCString(ctx, argv[0], &b);
    FILE *f = fopen(path, "rb"); if (!f) return JS_NULL;
    fseek(f, 0, SEEK_END); int size = ftell(f); fseek(f, 0, SEEK_SET);
    JSGCRef arr_ref; JSValue *arr = JS_PushGCRef(ctx, &arr_ref);
    *arr = JS_NewArray(ctx, size);
    for (int i = 0; i < size; i++) JS_SetPropertyUint32(ctx, *arr, i, JS_NewInt32(ctx, fgetc(f)));
    fclose(f);
    return JS_PopGCRef(ctx, &arr_ref);
}

static JSValue js_readFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf b; const char *path = JS_ToCString(ctx, argv[0], &b);
    FILE *f = fopen(path, "rb"); if (!f) return JS_NULL;
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1); size_t n = fread(buf, 1, len, f); buf[n] = '\0'; fclose(f);
    JSValue res = JS_NewString(ctx, buf); free(buf); return res;
}

static JSValue js_writeFile(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf b1, b2; const char *path = JS_ToCString(ctx, argv[0], &b1);
    size_t len; const char *content = JS_ToCStringLen(ctx, &len, argv[1], &b2);
    FILE *f = fopen(path, "wb"); if (!f) return JS_FALSE;
    fwrite(content, 1, len, f); fclose(f); return JS_TRUE;
}

static JSValue js_listFiles(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) {
    JSCStringBuf b; const char *path = JS_ToCString(ctx, argv[0], &b);
    DIR *d = opendir(path); if (!d) return JS_NULL;
    JSGCRef arr_ref; JSValue *arr = JS_PushGCRef(ctx, &arr_ref); *arr = JS_NewArray(ctx, 0);
    struct dirent *dir; int i = 0;
    while ((dir = readdir(d)) != NULL) { if (dir->d_name[0] == '.') continue; JS_SetPropertyUint32(ctx, *arr, i++, JS_NewString(ctx, dir->d_name)); }
    closedir(d); return JS_PopGCRef(ctx, &arr_ref);
}

// Stubs
static JSValue js_gc(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { JS_GC(ctx); return JS_UNDEFINED; }
static JSValue js_date_now(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return js_performance_now(ctx, this_val, argc, argv); }
static JSValue js_load(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_setTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }
static JSValue js_clearTimeout(JSContext *ctx, JSValue *this_val, int argc, JSValue *argv) { return JS_UNDEFINED; }

#include "useries_stdlib.h"

void run_app(const char *filename, size_t mem_kb, int use_raw_tty, const char *extra_arg) {
    size_t mem_size = mem_kb * 1024;
    uint8_t *mem_buf = malloc(mem_size);
    if (use_raw_tty) setup_term_raw();
    
    JSContext *ctx = JS_NewContext(mem_buf, mem_size, &useries_stdlib);
    if (extra_arg) {
        JSValue global = JS_GetGlobalObject(ctx);
        JS_SetPropertyStr(ctx, global, "ROM_PATH", JS_NewString(ctx, extra_arg));
    }
    
    FILE *f = fopen(filename, "rb");
    if (!f) { restore_term(); printf("File not found: %s\n", filename); return; }
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    char *src = malloc(len + 1); fread(src, 1, len, f); src[len] = '\0'; fclose(f);
    
    JSValue val = JS_Eval(ctx, src, len, filename, 0);
    if (JS_IsException(val)) {
        restore_term();
        printf("\x1b[31mCRASH in %s:\x1b[0m ", filename);
        JS_PrintValueF(ctx, JS_GetException(ctx), JS_DUMP_LONG);
        printf("\n");
    }
    
    JS_FreeContext(ctx);
    free(src); free(mem_buf);
    restore_term();
}

int main() {
    atexit(restore_term);
    while (1) {
        printf("\x1b[2J\x1b[H");
        printf("\x1b[1;36m========================================\n");
        printf("       THE µSERIES MULTIPLEXER\n");
        printf("========================================\x1b[0m\n\n");
        printf(" 1. \x1b[1;33mµGauntlet Pro\x1b[0m (16KB 3D Engine)\n");
        printf(" 2. \x1b[1;33mµStatic\x1b[0m      (Minimalist SSG)\n");
        printf(" 3. \x1b[1;33mµBrain\x1b[0m       (Brainfuck JIT)\n");
        printf(" 4. \x1b[1;33mµCHIP-8\x1b[0m      (12KB Emulator)\n");
        printf(" 5. \x1b[1;33mµGauntlet\x1b[0m    (Original Benchmarks)\n");
        printf(" q. Exit\n\n");
        printf("Select an option: ");
        fflush(stdout);
        
        char choice;
        if (scanf(" %c", &choice) != 1) break;
        if (choice == 'q') break;
        
        switch(choice) {
            case '1': run_app("world.js", 16, 1, NULL); break;
            case '2': run_app("ssg.js", 1024, 0, NULL); break;
            case '3': run_app("brain.js", 16, 0, NULL); break;
            case '4': run_app("chip8.js", 32, 1, "test.ch8"); break;
            case '5': run_app("benchmarks.js", 32, 0, NULL); break;
            default: printf("Invalid choice.\n"); sleep(1);
        }
        printf("\n\x1b[1;32mPress Enter to return to menu...\x1b[0m");
        getchar(); getchar();
    }
    return 0;
}
