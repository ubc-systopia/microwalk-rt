import sys

def mod_exp(x, e, n):
    y = 1
    for i in range(8):
        y = y * ((e & 1) * x | (~e & 1)) % n
        x = x * x % n
        e >>= 1
    return y

res = mod_exp(0x7, ord(sys.argv[0]), 0x8F)

print(res, file=sys.stderr);
