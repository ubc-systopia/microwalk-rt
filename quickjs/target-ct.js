function mod_exp(x, e, n) {
    let y = 1;
    for (let i = 0; i < 8; i++) {
        y = (y * (((e & 1) * x) | (~e & 1))) % n;
        x = (x * x) % n;
        e >>= 1;
    }
    return y;
}

export function test(arg) {
    return mod_exp(0x7, arg, 0x8f);
}
