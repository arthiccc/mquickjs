// Ultra-lite test
var r = get_rom(ROM_PATH);
if (r) {
    print("ROM Length: " + r.length);
} else {
    print("ROM Fail");
}

var i = 0;
while(i < 10) {
    draw(i, 10, 1);
    i++;
}
flush();
print("Lite Done");
