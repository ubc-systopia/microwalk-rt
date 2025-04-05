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

res = mod_exp(0x7, +scriptArgs[1], 0x8f);
