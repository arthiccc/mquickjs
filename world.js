// µGauntlet Pro: Hardcore 16KB Engine
// Wolfenstein-style DDA Raycaster

// 1. SIN/COS Lookup Tables (256 entries) to avoid Math.sin/cos allocation
var SIN = [];
var COS = [];
for (var i = 0; i < 256; i++) {
    var a = (i / 256) * Math.PI * 2;
    SIN[i] = Math.sin(a);
    COS[i] = Math.cos(a);
}

// 2. Map (8x8 grid encoded in two 32-bit ints if needed, or just bitwise)
// 1 = Wall, 0 = Empty
// 11111111
// 10000001
// 10110001
// 10000001
// 10011001
// 10000001
// 10000001
// 11111111
var MAP_LOW = 0x81B181FF;
var MAP_HIGH = 0xFF818199;

function getMap(x, y) {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return 1;
    var bit = (y * 8 + x);
    if (bit < 32) return (MAP_LOW >> bit) & 1;
    return (MAP_HIGH >> (bit - 32)) & 1;
}

// 3. Global State (Recycled variables to avoid GC)
var px = 3.5, py = 3.5, pa = 0;
var shades = " .:-=+*#%@";
var screenW = 80, screenH = 24;

function run() {
    while (true) {
        // Input
        var k = poll();
        if (key === 119) { px += COS[pa&255]*0.1; py += SIN[pa&255]*0.1; } // w
        if (key === 115) { px -= COS[pa&255]*0.1; py -= SIN[pa&255]*0.1; } // s
        if (key === 97) pa = (pa - 8) & 255; // a
        if (key === 100) pa = (pa + 8) & 255; // d
        if (key === 113) break; // q to quit

        // Render
        for (var x = 0; x < screenW; x++) {
            var rayA = (pa - 32 + (x / screenW) * 64) & 255;
            var rx = COS[rayA], ry = SIN[rayA];
            
            // DDA
            var dist = 0, hit = 0;
            while (!hit && dist < 10) {
                dist += 0.1;
                if (getMap(Math.floor(px + rx * dist), Math.floor(py + ry * dist))) hit = 1;
            }

            var wallH = Math.floor(screenH / dist);
            var wallTop = Math.floor((screenH - wallH) / 2);
            var wallBot = wallTop + wallH;
            
            var charIdx = Math.floor((1 - (dist / 10)) * 9);
            if (charIdx < 0) charIdx = 0;
            var c = shades.charCodeAt(charIdx);

            for (var y = 0; y < screenH; y++) {
                if (y >= wallTop && y < wallBot) {
                    set(x, y, c);
                } else if (y >= wallBot) {
                    set(x, y, 46); // . floor
                } else {
                    set(x, y, 32); // sky
                }
            }
        }
        flush();
    }
}

// Fix variables for global scope access
var key = 0;
function pollInput() {
    key = poll();
}

// The Main Loop
function loop() {
    pollInput();
    if (key === 113) return;
    
    // movement logic
    var moveStep = 0.15;
    if (key === 119) { px += COS[pa]*moveStep; py += SIN[pa]*moveStep; }
    if (key === 115) { px -= COS[pa]*moveStep; py -= SIN[pa]*moveStep; }
    if (key === 97) pa = (pa - 10) & 255;
    if (key === 100) pa = (pa + 10) & 255;

    // collision
    if (getMap(Math.floor(px), Math.floor(py))) {
        // basic bounce back
        px = 3.5; py = 3.5;
    }

    for (var x = 0; x < screenW; x++) {
        var angle = (pa - 40 + Math.floor((x / screenW) * 80)) & 255;
        var rX = COS[angle], rY = SIN[angle];
        var d = 0, h = 0;
        while (!h && d < 8) {
            d += 0.1;
            if (getMap(Math.floor(px + rX * d), Math.floor(py + rY * d))) h = 1;
        }
        var hW = Math.floor(screenH / d);
        var t = Math.floor((screenH - hW) / 2);
        var b = t + hW;
        var sIdx = Math.floor((1 - (d / 8)) * 9);
        var charC = shades.charCodeAt(sIdx < 0 ? 0 : sIdx);
        for (var y = 0; y < screenH; y++) {
            if (y < t) set(x, y, 32);
            else if (y < b) set(x, y, charC);
            else set(x, y, 46);
        }
    }
    flush();
    // In a real TTY engine we'd use a timer, but here we'll just loop
    // BUT we must avoid infinite loop that blocks the engine if not careful.
    // However, gauntlet_pro is a blocking call to JS_Eval.
}

// Optimization: Pre-bind functions to avoid lookup
var _set = set, _flush = flush, _poll = poll;

function start() {
    while(true) {
        loop();
        if (key === 113) break;
    }
}

start();
