// µGauntlet: Gamer Edition
function drawProgressBar(percent, width) {
    var filledWidth = Math.floor(percent * width);
    var bar = "[";
    for (var i = 0; i < width; i++) {
        bar += i < filledWidth ? "=" : " ";
    }
    bar += "]";
    return bar;
}

function runGamerBench() {
    print("\x1b[1;34m========================================");
    print("       µGAUNTLET v1.0 - GAMER EDITION");
    print("========================================\x1b[0m");
    
    var start = performance.now();
    var loops = 50000;
    for (var i = 0; i < loops; i++) {
        if (i % (loops / 10) === 0) {
            var progress = i / loops;
            print("Calculating Physics... " + drawProgressBar(progress, 20) + " " + Math.floor(progress * 100) + "%");
        }
        Math.sqrt(i) * Math.sin(i);
    }
    var end = performance.now();
    
    print("\n\x1b[1;32mBENCHMARK COMPLETE!\x1b[0m");
    print("Time: \x1b[1;37m" + (end - start) + "ms\x1b[0m");
    var score = Math.floor(1000000 / (end - start));
    print("Score: \x1b[1;33m" + score + " µMarks\x1b[0m");
    print("\x1b[1;34m========================================\x1b[0m");
}

runGamerBench();
