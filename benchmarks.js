// µGauntlet Benchmarks
var Gauntlet = {
    report: function(label, value) {
        print("[µGauntlet] " + label + ": " + value);
    }
};

function mandelbrot() {
    var width = 40;
    var height = 20;
    var max_iter = 100;
    var output = "";
    
    var start = performance.now();
    for (var y = 0; y < height; y++) {
        for (var x = 0; x < width; x++) {
            var c_re = (x - width/2) * 4.0/width;
            var c_im = (y - height/2) * 4.0/width;
            var x_re = 0, x_im = 0;
            var iter = 0;
            while (x_re*x_re + x_im*x_im <= 4 && iter < max_iter) {
                var x_new = x_re*x_re - x_im*x_im + c_re;
                x_im = 2*x_re*x_im + c_im;
                x_re = x_new;
                iter++;
            }
            output += iter === max_iter ? "#" : ".";
        }
        output += "\n";
    }
    var end = performance.now();
    
    print(output);
    Gauntlet.report("Mandelbrot Time", (end - start) + "ms");
}

function raycaster() {
    // Ultra-tiny 2D Raycaster rendered to ASCII
    var screenWidth = 40;
    var screenHeight = 20;
    var map = [
        1,1,1,1,1,1,1,1,
        1,0,0,0,0,0,0,1,
        1,0,1,0,0,1,0,1,
        1,0,0,0,0,0,0,1,
        1,0,1,1,0,1,0,1,
        1,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,1,
        1,1,1,1,1,1,1,1
    ];
    var mapSize = 8;
    var playerX = 3.5, playerY = 3.5, playerA = 0.5;
    var output = "";

    var start = performance.now();
    for (var x = 0; x < screenWidth; x++) {
        var rayAngle = (playerA - 0.6) + (x / screenWidth) * 1.2;
        var distanceToWall = 0;
        var hitWall = false;
        
        var eyeX = Math.sin(rayAngle);
        var eyeY = Math.cos(rayAngle);
        
        while (!hitWall && distanceToWall < 16) {
            distanceToWall += 0.1;
            var testX = Math.floor(playerX + eyeX * distanceToWall);
            var testY = Math.floor(playerY + eyeY * distanceToWall);
            
            if (testX < 0 || testX >= mapSize || testY < 0 || testY >= mapSize) {
                hitWall = true;
                distanceToWall = 16;
            } else if (map[testY * mapSize + testX] === 1) {
                hitWall = true;
            }
        }
        
        var ceiling = screenHeight / 2.0 - screenHeight / distanceToWall;
        var floor = screenHeight - ceiling;
        
        // This is a column-based renderer, but we need row-based for ASCII
        // So we'll actually just calculate for each pixel (slow but fits memory)
    }
    
    // Better ASCII Raycaster for Row-major output
    output = "";
    for (var y = 0; y < screenHeight; y++) {
        for (var x = 0; x < screenWidth; x++) {
            var rayAngle = (playerA - 0.6) + (x / screenWidth) * 1.2;
            var eyeX = Math.sin(rayAngle);
            var eyeY = Math.cos(rayAngle);
            var dist = 0;
            var hit = false;
            while(!hit && dist < 8) {
                dist += 0.1;
                var tx = Math.floor(playerX + eyeX * dist);
                var ty = Math.floor(playerY + eyeY * dist);
                if(map[ty*mapSize+tx]) hit = true;
            }
            var wallH = screenHeight / dist;
            if (y > screenHeight/2 - wallH/2 && y < screenHeight/2 + wallH/2) {
                output += dist < 3 ? "@" : (dist < 5 ? "x" : ".");
            } else {
                output += " ";
            }
        }
        output += "\n";
    }
    var end = performance.now();
    print(output);
    Gauntlet.report("Raycaster Time", (end - start) + "ms");
}

print("\x1b[1;35m--- BOSS 1: MANDELBROT SLAYER ---\x1b[0m");
mandelbrot();
print("\x1b[1;35m--- BOSS 2: RAYCASTER REALM ---\x1b[0m");
raycaster();
print("\x1b[1;32m--- GAUNTLET COMPLETE ---\x1b[0m");
