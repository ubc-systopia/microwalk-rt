import sys

def mod_exp(x, e, n):
    y = 1
    while e > 0:
        if e % 2 == 0:
            x = (x * x) % n
            e = e // 2
        else:
            y = (x * y) % n
            e = e - 1
    return y

res = mod_exp(0x7, ord(sys.argv[0]), 0x8F)

print(res, file=sys.stderr);
