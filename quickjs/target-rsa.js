function mod_exp(x, e, n) {
    let y = 1;
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

export function test(arg) {
    return mod_exp(0x7, arg, 0x8f);
}
