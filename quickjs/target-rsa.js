function mod_exp(x, e, n) {
    y = 1;
    while (e > 0) {
        if (e % 2 == 0) {
            y = (x * x) % n;
            e /= 2;
        } else {
            x = (x * y) % n;
            e -= 2;
        }
    }
    return y;
}

function test(arg) {
    res = mod_exp(0x7, arg, 0x8f);
}
