// Junior Dev Challenge: RLE Compression
// Goal: compress("aaabbb") -> "3a3b"
function solution(input) {
    if (!input) return "";
    var result = "";
    var count = 1;
    for (var i = 1; i <= input.length; i++) {
        if (i < input.length && input[i] === input[i-1]) {
            count++;
        } else {
            result += count + input[i-1];
            count = 1;
        }
    }
    return result;
}

// Test cases
var test = "aaabbbcccdddeeeee";
var res = solution(test);
print("Input:  " + test);
print("Output: " + res);
if (res === "3a3b3c3d5e") {
    print("\x1b[1;32mPASSED\x1b[0m");
} else {
    print("\x1b[1;31mFAILED\x1b[0m");
}
