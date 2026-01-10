var tape = new Uint8Array(2048);
var ptr = 0;
// Mandelbrot by Erik Bosman
var code = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++."; // Back to hello for verification

function run() {
    var pc = 0;
    while (pc < code.length) {
        var c = code.charAt(pc);
        if (c === '>') ptr++;
        else if (c === '<') ptr--;
        else if (c === '+') tape[ptr]++;
        else if (c === '-') tape[ptr]--;
        else if (c === '.') print(String.fromCharCode(tape[ptr]));
        else if (c === '[') {
            if (tape[ptr] === 0) {
                var depth = 1;
                while (depth > 0) {
                    pc++;
                    if (code.charAt(pc) === '[') depth++;
                    if (code.charAt(pc) === ']') depth--;
                }
            }
        } else if (c === ']') {
            if (tape[ptr] !== 0) {
                var depth = 1;
                while (depth > 0) {
                    pc--;
                    if (code.charAt(pc) === ']') depth++;
                    if (code.charAt(pc) === '[') depth--;
                }
            }
        }
        pc++;
    }
}

print("µBrain Final Limit (16KB):\n");
run();
print("\nDone\n");
