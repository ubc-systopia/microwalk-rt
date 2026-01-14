function mod_exp(x, e, n) {
    y = 1;
    for (let i = 0; i < 8; i++) {
        y = (y * (((e & 1) * x) | (~e & 1))) % n;
        x = (x * x) % n;
        e >>= 1;
    }
    return y;
}

function test(arg) {
    res = mod_exp(0x7, arg, 0x8f);
}
